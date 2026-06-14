/**
 * @file    DatabaseCommon.h
 * @brief   数据存储模块公共定义 —— 定长二进制记录结构体
 * @author  成员3（数据存储模块）
 * @date    2026-05-21
 *
 * @details
 * 本文件定义了整个数据存储层的核心数据结构 FocusRecord。
 * 设计约束：
 *   - 严禁使用任何第三方库（SQLite、JSON、XML、Boost 等），仅使用 C++ 标准库。
 *   - 采用"定长二进制文件（Fixed-length Binary File）"进行持久化存储，
 *     每条记录在磁盘上占据固定字节数，以便通过记录索引（recordId）
 *     直接计算文件偏移量（Offset）进行随机读写与原地更新。
 *   - 使用 #pragma pack(push, 1) 消除编译器自动添加的内存对齐填充字节，
 *     确保 sizeof(FocusRecord) 在内存和磁盘上严格一致。
 *
 * 结构体大小计算（单位：字节）：
 *   int recordId          =  4
 *   char plantType[32]    = 32
 *   int durationSeconds   =  4
 *   long long timestamp   =  8
 *   bool isSuccess        =  1
 *   char reserve[15]      = 15  (预留填充，使总大小精确为 64)
 *   ─────────────────────────
 *   总计                  = 64 字节
 *
 * @note
 * 64 字节是 2 的幂（2^6），有利于：
 *   - 偏移量计算简洁：Offset = recordId << 6（等价于 recordId * 64）
 *   - 与磁盘扇区/缓存行对齐，提高读写效率
 *
 * @attention
 * 严禁增删或调整成员顺序，否则 sizeof(FocusRecord) 将发生变化，
 * 导致已有数据库文件无法正确读取（偏移量错位）。
 */

#ifndef DATABASECOMMON_H
#define DATABASECOMMON_H

// ────────────────────────────────────────────────────────────
// 消除结构体字节对齐填充，确保跨平台/跨编译器 sizeof 一致
// ────────────────────────────────────────────────────────────
#pragma pack(push, 1)

/**
 * @struct FocusRecord
 * @brief  专注记录 —— 定长二进制文件中的基本存储单元。
 *
 * 每条记录代表用户完成（或失败）的一次专注会话。
 * 结构体在磁盘上的大小严格固定为 64 字节（sizeof(FocusRecord) == 64），
 * 因此第 N 条记录的起始偏移量 = N * 64。
 *
 * @note
 * - recordId 即为该记录在二进制文件中的逻辑索引（0-based），
 *   由 DatabaseManager::writeRecord() 在写入前自动赋值。
 * - reserve 数组为预留填充字节，用于未来扩展字段时保持向后兼容。
 *   当前未使用，写入时须填充零值。
 */
struct FocusRecord
{
    /** @brief 记录唯一标识（0-based 索引），对应二进制文件中的顺序位置 */
    int recordId;

    /** @brief 种植植物的名称，定长 32 字符（不足部分以 '\0' 填充） */
    char plantType[32];

    /** @brief 本次专注的时长，单位：秒 */
    int durationSeconds;

    /** @brief 专注结束时的 Unix 时间戳（自 1970-01-01 00:00:00 UTC 起的秒数） */
    long long timestamp;

    /** @brief 专注是否成功完成（true = 成功，树长大；false = 失败，树枯萎） */
    bool isSuccess;

    /**
     * @brief 预留填充字节数组，使 sizeof(FocusRecord) 精确等于 64 字节。
     * @details
     * 当前保留供未来扩展（如增加标签、备注、评分等字段）。
     * 每次写入记录时，本数组必须全部置零，以确保二进制文件内容可重现。
     */
    char reserve[15];
};

#pragma pack(pop)

// ────────────────────────────────────────────────────────────
// 编译期静态断言：确保结构体大小严格为 64 字节
// ────────────────────────────────────────────────────────────
static_assert(sizeof(FocusRecord) == 64,
    "FocusRecord 大小必须严格为 64 字节。请勿增删或调整成员顺序。");

#endif // DATABASECOMMON_H
