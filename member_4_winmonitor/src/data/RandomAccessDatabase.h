#ifndef RANDOMACCESSDATABASE_H
#define RANDOMACCESSDATABASE_H

#include <QString>
#include <fstream>
#include <vector>
#include "core/FocusRecord.h"

/**
 * @brief 随机文件数据库管理器
 *
 * 核心功能（课程要求：Random file processing — writing, reading, and updating）：
 * - writeRecord():  追加写入（文件末尾）
 * - readRecord():   随机读取（seekg 定位）
 * - updateRecord(): 随机更新（seekp 定位覆盖）
 * - deleteRecord(): 逻辑删除（标记 isSuccess = 0）
 *
 * 文件格式：
 * - 二进制定长记录，每条 64 字节（sizeof(FocusRecord) == 64）
 * - 首 4 字节存储总记录数（int32_t），随后紧跟记录数据
 *
 * 偏移量计算公式：
 *   recordOffset = sizeof(int32_t) + index * sizeof(FocusRecord)
 */
class RandomAccessDatabase {
public:
    /**
     * @brief 构造函数
     * @param filePath 二进制文件路径
     */
    explicit RandomAccessDatabase(const QString& filePath);

    ~RandomAccessDatabase();

    /// 打开或创建文件，返回是否成功
    bool open();

    /// 关闭文件
    void close();

    /// 文件是否已打开
    bool isOpen() const { return m_file.is_open(); }

    // ─── 增删改查接口 ──────────────────────────────────────

    /**
     * @brief 追加写入一条记录
     * @param record 待写入记录
     * @return true 写入成功
     *
     * 算法步骤：
     * 1. seekp(0, end) 定位到文件末尾
     * 2. write(&record, sizeof(FocusRecord))
     * 3. 更新文件头的记录计数
     */
    bool writeRecord(const FocusRecord& record);

    /**
     * @brief 随机读取第 index 条记录（0-based）
     * @param index  记录索引
     * @param record 输出参数
     * @return true 读取成功
     *
     * 算法步骤：
     * 1. 校验 index 合法性
     * 2. offset = HEADER_SIZE + index * RECORD_SIZE
     * 3. seekg(offset) → read(&record, RECORD_SIZE)
     */
    bool readRecord(int index, FocusRecord& record) const;

    /**
     * @brief 随机更新第 index 条记录
     * @param index  记录索引
     * @param record 新数据
     * @return true 更新成功
     *
     * 算法步骤：
     * 1. 校验 index 合法性
     * 2. offset = HEADER_SIZE + index * RECORD_SIZE
     * 3. seekp(offset) → write(&record, RECORD_SIZE)
     */
    bool updateRecord(int index, const FocusRecord& record);

    /**
     * @brief 逻辑删除第 index 条记录（标记为失败/枯萎）
     * @param index 记录索引
     * @return true 删除成功
     */
    bool deleteRecord(int index);

    /// 获取当前总记录数
    int recordCount() const { return m_recordCount; }

    /// 读取所有记录到 vector
    std::vector<FocusRecord> readAll() const;

    /// 文件路径
    const QString& filePath() const { return m_filePath; }

private:
    /// 文件头大小（存储 int32_t 记录计数）
    static constexpr int HEADER_SIZE = sizeof(int32_t);
    /// 单条记录大小
    static constexpr int RECORD_SIZE = sizeof(FocusRecord);

    /// 从文件头同步记录计数
    void loadHeader();
    /// 更新文件头中的记录计数
    void saveHeader();

    QString          m_filePath;
    mutable std::fstream m_file;  // mutable 允许 const 方法读
    int              m_recordCount;
};

#endif // RANDOMACCESSDATABASE_H
