/**
 * @file    DatabaseManager.cpp
 * @brief   定长二进制文件数据库管理器 —— 实现
 * @author  成员3（数据存储模块）
 * @date    2026-05-21
 *
 * @details
 * 本文件实现了 DatabaseManager 类的所有成员函数。
 * 所有 I/O 操作均采用二进制模式（std::ios::binary），
 * 配合 seekg/seekp 实现索引级别的随机定位读写。
 *
 * 关键实现细节：
 *   - 文件初始化采用"先创建后打开"策略，解决 std::fstream
 *     无法以 in|out 模式打开不存在文件的问题。
 *   - 每次 seek 前调用 file.clear() 清除可能残留的 EOF/Fail 标志。
 *   - 随机读写前进行严格的索引越界检查（readRecord / updateRecord）。
 *   - updateRecord 额外校验 newRecord.recordId == recordId 参数一致性。
 *   - writeRecord 自动生成 recordId（= 当前记录总数），
 *     并将 reserve 字段清零以保证可重现性。
 */

#include "DatabaseManager.h"
#include <cstring>

// ================================================================
// 构造函数 & 析构函数
// ================================================================

/**
 * @brief 构造函数——保存数据库文件路径，初始化成员状态。
 *
 * @details
 * 不在构造函数中执行任何 I/O 操作，以避免因文件不存在或权限不足
 * 而导致构造失败。实际的文件初始化由 initialize() 完成。
 *
 * @param dbPath 二进制数据库文件的完整路径。
 */
DatabaseManager::DatabaseManager(const std::string& dbPath)
    : m_dbPath(dbPath)
    , m_initialized(false)
{
}

/**
 * @brief 析构函数——若文件流已打开则自动关闭。
 *
 * @details
 * 调用 m_file.close() 释放文件句柄。
 * 若 m_file 未关联任何文件，close() 为无操作。
 */
DatabaseManager::~DatabaseManager()
{
    if (m_file.is_open())
    {
        m_file.close();
    }
}

// ================================================================
// initialize() —— 初始化数据库文件
// ================================================================

/**
 * @brief 初始化数据库文件，确保文件存在并处于可读写状态。
 *
 * @details
 * 问题背景：
 *   C++ 标准中，以 std::ios::in | std::ios::out 模式打开一个不存在的文件
 *   会失败（不会自动创建）。因此需要两阶段策略。
 *
 * 算法步骤：
 *   1. 直接尝试以 std::ios::in | std::ios::out | std::ios::binary 模式打开。
 *   2. 若步骤 1 成功 → 文件已存在且可读写，初始化完成。
 *   3. 若步骤 1 失败 → 按以下过程执行"先创建后打开"：
 *      a. 以 std::ios::out | std::ios::binary 模式打开，
 *         此模式会自动创建新文件（若不存在）或截断已有文件（若存在）。
 *      b. 立即关闭该文件流（空文件已存在于磁盘上）。
 *      c. 再次以 std::ios::in | std::ios::out | std::ios::binary 模式打开。
 *   4. 若步骤 3 成功 → 初始化完成。
 *   5. 若步骤 3 仍失败 → 返回 false（如路径无效、目录不存在、权限不足）。
 *
 * @return true  文件就绪，后续读写操作可执行。
 * @return false 初始化失败。
 */
bool DatabaseManager::initialize()
{
    // 第 1 次尝试：直接以读写模式打开（文件已存在的情况）
    m_file.open(m_dbPath, std::ios::in | std::ios::out | std::ios::binary);

    if (m_file.is_open())
    {
        m_initialized = true;
        return true;
    }

    // 第 2 步：文件不存在，先创建再打开
    // 以纯输出模式创建空文件（若文件已存在，此操作会将其截断为空，
    // 但由于第 1 步已经确认文件不存在，所以这里不会误截断）
    {
        std::ofstream creator(m_dbPath, std::ios::out | std::ios::binary);
        if (!creator.is_open())
        {
            return false;   // 连创建文件都不行（路径无效 / 权限不足）
        }
        creator.close();    // 空文件已写入磁盘
    }

    // 第 3 步：重新以读写模式打开刚刚创建的空文件
    m_file.open(m_dbPath, std::ios::in | std::ios::out | std::ios::binary);

    if (m_file.is_open())
    {
        m_initialized = true;
        return true;
    }

    return false;
}

// ================================================================
// writeRecord(FocusRecord& record) —— 追加写入
// ================================================================

/**
 * @brief 追加一条记录到文件末尾。
 *
 * @details
 * 算法步骤：
 *   1. 检查 m_initialized 及 m_file.is_open()，未就绪则返回 false。
 *   2. 获取当前记录总数 count = getRecordCount()。
 *   3. 设置 record.recordId = count（自动生成连续索引）。
 *   4. 将 record.reserve 数组全部置零（memset），确保填充数据一致。
 *   5. 清除文件流状态标志（file.clear()），预防之前的 EOF 标志干扰。
 *   6. 将写指针移动到文件末尾：file.seekp(0, std::ios::end)。
 *   7. 写入 sizeof(FocusRecord) 字节：file.write(...)。
 *   8. 强制刷新到磁盘：file.flush()。
 *
 * @param[in,out] record 待写入的记录。
 *                       函数返回后，record.recordId 已被赋值为新索引。
 * @return true  写入成功。
 * @return false 写入失败。
 */
bool DatabaseManager::writeRecord(FocusRecord& record)
{
    if (!m_initialized || !m_file.is_open())
    {
        return false;
    }

    // 自动分配 recordId = 当前记录总数
    record.recordId = getRecordCount();

    // 将预留字段清零，保证二进制内容一致
    std::memset(record.reserve, 0, sizeof(record.reserve));

    // 清除可能的错误状态标志（如之前操作触发的 EOF）
    m_file.clear();

    // 将写指针定位到文件末尾，准备追加
    m_file.seekp(0, std::ios::end);
    if (!m_file.good())
    {
        return false;
    }

    // 执行写入
    m_file.write(reinterpret_cast<const char*>(&record), sizeof(FocusRecord));
    if (!m_file.good())
    {
        return false;
    }

    // 强制刷新缓冲区，确保数据持久化到磁盘
    m_file.flush();

    return true;
}

// ================================================================
// readRecord(int recordId, FocusRecord& outRecord) —— 随机读取
// ================================================================

/**
 * @brief 根据 recordId 随机定位并读取指定记录。
 *
 * @details
 * 算法步骤：
 *   1. 检查 m_initialized 及 m_file.is_open()，未就绪则返回 false。
 *   2. 调用 getRecordCount() 获取总记录数 N。
 *   3. 若 recordId < 0 或 recordId >= N → 索引越界，返回 false。
 *   4. 调用 m_file.clear() 清除可能残留的 EOF / failbit 标志。
 *   5. 计算文件偏移量：offset = recordId * sizeof(FocusRecord)。
 *   6. 将读指针定位到 offset：m_file.seekg(offset, std::ios::beg)。
 *   7. 检查 seekg 是否成功（m_file.good()），失败则返回 false。
 *   8. 执行读取：m_file.read(reinterpret_cast<char*>(&outRecord), sizeof(FocusRecord))。
 *   9. 检查 read 是否成功（m_file.good()），失败则返回 false。
 *
 * @param[in]  recordId  目标记录索引（0-based）。
 * @param[out] outRecord 读取到的记录数据（仅在成功时有效）。
 * @return true  读取成功，outRecord 包含有效数据。
 * @return false 读取失败（未初始化、越界、I/O 错误）。
 */
bool DatabaseManager::readRecord(int recordId, FocusRecord& outRecord)
{
    if (!m_initialized || !m_file.is_open())
    {
        return false;
    }

    // 越界检查
    int total = getRecordCount();
    if (recordId < 0 || recordId >= total)
    {
        return false;
    }

    // 清除流状态（如上次读取触发的 EOF），否则 seekg 可能无效
    m_file.clear();

    // 计算偏移量并定位读指针
    std::streamoff offset = static_cast<std::streamoff>(recordId) * sizeof(FocusRecord);
    m_file.seekg(offset, std::ios::beg);
    if (!m_file.good())
    {
        return false;
    }

    // 执行读取
    m_file.read(reinterpret_cast<char*>(&outRecord), sizeof(FocusRecord));
    if (!m_file.good())
    {
        return false;
    }

    return true;
}

// ================================================================
// updateRecord(int recordId, const FocusRecord& newRecord) —— 原地更新
// ================================================================

/**
 * @brief 根据 recordId 随机定位并原地覆写记录。
 *
 * @details
 * 原地更新（In-place Update）的核心优势：
 *   不改变文件大小、不移动其他记录，仅将目标位置的旧数据直接覆盖为新数据。
 *   这避免了"删除-重新写入"带来的文件碎片和性能开销。
 *
 * 算法步骤：
 *   1. 检查 m_initialized 及 m_file.is_open()，未就绪则返回 false。
 *   2. 调用 getRecordCount() 获取总记录数 N。
 *   3. 若 recordId < 0 或 recordId >= N → 索引越界，返回 false。
 *   4. 校验 newRecord.recordId == recordId，不匹配则返回 false
 *      （防止调用者传入错误的 recordId，破坏索引一致性）。
 *   5. 调用 m_file.clear() 清除可能残留的 EOF / failbit 标志。
 *   6. 计算文件偏移量：offset = recordId * sizeof(FocusRecord)。
 *   7. 将写指针定位到 offset：m_file.seekp(offset, std::ios::beg)。
 *   8. 检查 seekp 是否成功（m_file.good()），失败则返回 false。
 *   9. 执行覆写：m_file.write(reinterpret_cast<const char*>(&newRecord), sizeof(FocusRecord))。
 *  10. 检查 write 是否成功（m_file.good()），失败则返回 false。
 *  11. 强制刷新：m_file.flush()，确保数据持久化。
 *
 * @param[in] recordId   目标记录索引（0-based）。
 * @param[in] newRecord  新记录数据，其 recordId 必须等于 recordId 参数。
 * @return true  更新成功。
 * @return false 更新失败（未初始化、越界、ID 不匹配、I/O 错误）。
 */
bool DatabaseManager::updateRecord(int recordId, const FocusRecord& newRecord)
{
    if (!m_initialized || !m_file.is_open())
    {
        return false;
    }

    // 越界检查
    int total = getRecordCount();
    if (recordId < 0 || recordId >= total)
    {
        return false;
    }

    // 索引一致性校验：确保传入的记录 ID 与参数一致
    if (newRecord.recordId != recordId)
    {
        return false;
    }

    // 清除流状态标志
    m_file.clear();

    // 计算偏移量并定位写指针
    std::streamoff offset = static_cast<std::streamoff>(recordId) * sizeof(FocusRecord);
    m_file.seekp(offset, std::ios::beg);
    if (!m_file.good())
    {
        return false;
    }

    // 执行原地覆写
    m_file.write(reinterpret_cast<const char*>(&newRecord), sizeof(FocusRecord));
    if (!m_file.good())
    {
        return false;
    }

    // 强制刷新到磁盘
    m_file.flush();

    return true;
}

// ================================================================
// getAllRecords() —— 全表顺序读取
// ================================================================

/**
 * @brief 顺序读取文件中所有记录并返回向量。
 *
 * @details
 * 算法步骤：
 *   1. 创建空的 std::vector<FocusRecord> result。
 *   2. 若未初始化或文件未打开，返回空向量。
 *   3. 调用 m_file.clear() 清除可能的错误状态。
 *   4. 将读指针置于文件开头：m_file.seekg(0, std::ios::beg)。
 *   5. 循环执行：
 *      a. 创建临时 FocusRecord temp。
 *      b. 调用 m_file.read() 读取 sizeof(FocusRecord) 字节。
 *      c. 若读取成功（file.good()）→ push_back(temp)。
 *      d. 若读取失败 → 跳出循环（文件结束或 I/O 错误）。
 *   6. 返回 result。
 *
 * @note
 * 空文件或文件仅含部分不完整记录时，返回空向量或已成功读取的部分记录。
 *
 * @return std::vector<FocusRecord> 包含所有有效记录的向量。
 */
std::vector<FocusRecord> DatabaseManager::getAllRecords()
{
    std::vector<FocusRecord> result;

    if (!m_initialized || !m_file.is_open())
    {
        return result;
    }

    // 清除可能残留的错误标志，从文件头开始顺序读取
    m_file.clear();
    m_file.seekg(0, std::ios::beg);

    while (true)
    {
        FocusRecord temp;
        m_file.read(reinterpret_cast<char*>(&temp), sizeof(FocusRecord));

        if (m_file.good())
        {
            result.push_back(temp);
        }
        else
        {
            // EOF 或读取错误，终止循环
            break;
        }
    }

    // 重置 EOF 标志以便后续操作
    m_file.clear();

    return result;
}

// ================================================================
// getRecordCount() —— 记录总数（O(1) 算法）
// ================================================================

/**
 * @brief 获取数据库文件中的记录总数。
 *
 * @details
 * 算法原理（O(1) 时间复杂度）：
 *   记录总数 = 文件总字节数 / 每条记录的固定字节数
 *   = fileSize / sizeof(FocusRecord)
 *   = fileSize / 64
 *
 * 由于每条记录固定 64 字节，且所有记录通过追加写入连续存储
 * （不产生空洞、不删除中间记录），文件大小必定是 64 的整数倍。
 * 因此除法结果即为精确的记录总数。
 *
 * 实现细节：
 *   1. 若未初始化或文件未打开 → 返回 0。
 *   2. 在保存当前读/写指针位置后，调用 seekg 定位到文件末尾获取大小。
 *   3. 文件大小 / sizeof(FocusRecord) 取整。
 *   4. 恢复读/写指针（实际实现中 seekg(0, end) 后需重新定位）。
 *
 * @return int 记录总数（>= 0）。
 */
int DatabaseManager::getRecordCount()
{
    if (!m_initialized || !m_file.is_open())
    {
        return 0;
    }

    // 保存当前读指针位置，稍后恢复
    std::streampos oldPos = m_file.tellg();

    m_file.clear();

    // 将读指针移到文件末尾以获取文件大小
    m_file.seekg(0, std::ios::end);
    std::streampos fileSize = m_file.tellg();

    // 恢复读指针位置
    m_file.seekg(oldPos, std::ios::beg);
    m_file.clear();

    // 记录总数 = 文件大小 / 单条记录大小
    return static_cast<int>(fileSize) / static_cast<int>(sizeof(FocusRecord));
}
