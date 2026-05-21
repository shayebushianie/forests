/**
 * @file    DatabaseManager.h
 * @brief   定长二进制文件数据库管理器 —— 声明
 * @author  成员3（数据存储模块）
 * @date    2026-05-21
 *
 * @details
 * DatabaseManager 是本数据存储模块的核心类，封装了对定长二进制文件
 * 的 CRUD 操作（追加写入、随机读取、原地更新、全表查询、计数）。
 *
 * 设计原则：
 *   - 零外部依赖：仅使用 C++ 标准库 <fstream>、<vector>、<string>。
 *   - 定长记录：每条 FocusRecord 在磁盘上严格占 64 字节，
 *     偏移量 = recordId * sizeof(FocusRecord)。
 *   - 健壮性优先：每次文件操作前后均进行状态检查与恢复。
 *   - 索引一致性：recordId 由 writeRecord() 自动赋值，
 *     确保记录 ID 与其在文件中的物理位置一致（第 0 条 → 偏移 0；第 N 条 → 偏移 N*64）。
 *
 * 文件生命周期：
 *   1. 构造：传入文件路径，不执行任何 I/O。
 *   2. initialize()：确保文件存在且以读写模式打开。
 *      - 若文件不存在 → 先以 std::ios::out 创建空文件。
 *      - 再以 std::ios::in | std::ios::out | std::ios::binary 模式打开。
 *   3. 读写操作（writeRecord / readRecord / updateRecord / getAllRecords）。
 *   4. 析构：自动关闭文件流。
 *
 * @attention
 * - 本类非线程安全，如需多线程访问，调用方须自行加锁。
 * - updateRecord() 要求 newRecord 的 recordId 与参数 recordId 一致，
 *   否则操作将被拒绝。
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "DatabaseCommon.h"
#include <fstream>
#include <string>
#include <vector>

/**
 * @class DatabaseManager
 * @brief 定长二进制数据库管理器。
 *
 * 提供对 FocusRecord 二进制文件的追加写入、随机定位读取、
 * 随机定位原地更新、全表顺序读取及记录计数等功能。
 *
 * @note 使用前必须调用 initialize() 成功后才能执行数据操作。
 * @note 每次 I/O 操作返回 bool 指示成功/失败，不抛出异常。
 */
class DatabaseManager
{
public:
    /**
     * @brief 构造函数——仅保存文件路径，不执行任何 I/O 操作。
     * @param dbPath 二进制数据库文件的完整路径（含文件名）。
     *               若路径中的目录不存在，initialize() 将失败。
     */
    DatabaseManager(const std::string& dbPath);

    /**
     * @brief 析构函数——自动关闭文件流（若已打开）。
     */
    ~DatabaseManager();

    // ────────────────────────────────────────────────────────
    // 初始化
    // ────────────────────────────────────────────────────────

    /**
     * @brief 初始化数据库文件，确保文件存在且处于可读写状态。
     *
     * @details
     * 算法步骤：
     *   1. 尝试以 std::ios::in | std::ios::out | std::ios::binary 打开文件。
     *   2. 若打开失败（文件不存在），先以 std::ios::out | std::ios::binary
     *      创建空文件，再重新以读写模式打开。
     *   3. 若两次尝试均失败，返回 false。
     *
     * @return true  初始化成功，文件已就绪。
     * @return false 初始化失败（如路径无效、权限不足）。
     */
    bool initialize();

    // ────────────────────────────────────────────────────────
    // CRUD 操作
    // ────────────────────────────────────────────────────────

    /**
     * @brief 追加一条记录到文件末尾（Append）。
     *
     * @details
     * 算法步骤：
     *   1. 检查文件流是否已打开且处于有效状态，否则返回 false。
     *   2. 获取当前记录总数 count = getRecordCount()。
     *   3. 将 record.recordId 赋值为 count（新记录的索引）。
     *   4. 将 reserve 字段全部置零，确保填充数据一致。
     *   5. 将文件写指针移动到文件末尾：seekp(0, std::ios::end)。
     *   6. 调用 file.write() 写入 sizeof(FocusRecord) 字节。
     *   7. 调用 file.flush() 确保数据落盘。
     *
     * @param[in,out] record 待写入的记录引用。
     *                       函数会自动修改其 recordId 和 reserve。
     * @return true  写入成功。
     * @return false 写入失败（流未就绪、磁盘满等）。
     */
    bool writeRecord(FocusRecord& record);

    /**
     * @brief 根据 recordId 随机定位并读取指定记录。
     *
     * @details
     * 算法步骤：
     *   1. 检查文件流是否已打开，否则返回 false。
     *   2. 检查 recordId 是否在有效范围 [0, getRecordCount() - 1] 内，越界返回 false。
     *   3. 清除文件流状态标志（如之前的 EOF 标志）——调用 file.clear()。
     *   4. 计算偏移量 offset = recordId * sizeof(FocusRecord)。
     *   5. 将读指针移动到 offset：file.seekg(offset, std::ios::beg)。
     *   6. 检查 seekg 是否成功，失败则返回 false。
     *   7. 调用 file.read() 读取 sizeof(FocusRecord) 字节到 outRecord。
     *   8. 检查读取是否成功（file.good()），失败则返回 false。
     *
     * @param[in]  recordId  要读取的记录索引（0-based）。
     * @param[out] outRecord 读取到的记录数据。
     * @return true  读取成功。
     * @return false 读取失败（越界、I/O 错误等）。
     */
    bool readRecord(int recordId, FocusRecord& outRecord);

    /**
     * @brief 根据 recordId 随机定位并原地更新记录（Overwrite）。
     *
     * @details
     * 原地更新是指不改变文件大小、不移动其他记录，
     * 直接将新数据覆写到旧记录的物理位置。
     *
     * 算法步骤：
     *   1. 检查文件流是否已打开，否则返回 false。
     *   2. 检查 recordId 是否在有效范围 [0, getRecordCount() - 1] 内，越界返回 false。
     *   3. 验证 newRecord.recordId == recordId，不一致则返回 false。
     *   4. 清除文件流状态标志——调用 file.clear()。
     *   5. 计算偏移量 offset = recordId * sizeof(FocusRecord)。
     *   6. 将写指针移动到 offset：file.seekp(offset, std::ios::beg)。
     *   7. 检查 seekp 是否成功，失败则返回 false。
     *   8. 调用 file.write() 覆写 sizeof(FocusRecord) 字节。
     *   9. 调用 file.flush() 确保数据落盘。
     *
     * @param[in] recordId   要更新的记录索引（0-based）。
     * @param[in] newRecord  新的记录数据（其 recordId 必须与 recordId 参数一致）。
     * @return true  更新成功。
     * @return false 更新失败（越界、ID 不匹配、I/O 错误等）。
     */
    bool updateRecord(int recordId, const FocusRecord& newRecord);

    /**
     * @brief 顺序读取文件中的所有记录。
     *
     * @details
     * 算法步骤：
     *   1. 清除文件流状态标志——调用 file.clear()。
     *   2. 将读指针移动到文件开头：file.seekg(0, std::ios::beg)。
     *   3. 循环调用 file.read()，每次读取 sizeof(FocusRecord) 字节，
     *      直到 file.eof() 为 true 或读取失败。
     *   4. 将所有成功读取的记录加入 std::vector 并返回。
     *
     * @return std::vector<FocusRecord> 包含所有记录的向量。
     *                                  若文件为空或无有效记录，返回空向量。
     */
    std::vector<FocusRecord> getAllRecords();

    // ────────────────────────────────────────────────────────
    // 统计
    // ────────────────────────────────────────────────────────

    /**
     * @brief 获取当前数据库文件中的总记录数。
     *
     * @details
     * 算法原理：
     *   利用文件大小除以单条记录大小来计算记录数：
     *     recordCount = fileSize / sizeof(FocusRecord)
     *   由于每条记录固定 64 字节，且追加写入不产生空洞，
     *   此方法可在 O(1) 时间内得到精确的记录总数。
     *
     * @return int 记录总数。若文件为空、未初始化或发生错误，返回 0。
     */
    int getRecordCount();

private:
    /** @brief 数据库文件路径 */
    std::string m_dbPath;

    /** @brief 二进制文件流（同时支持读写） */
    std::fstream m_file;

    /** @brief 标记是否已成功初始化（initialize() 已调用且成功） */
    bool m_initialized;
};

#endif // DATABASEMANAGER_H
