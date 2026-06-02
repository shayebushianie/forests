#ifndef FOCUSRECORD_H
#define FOCUSRECORD_H

#include <cstdint>
#include <cstring>
#include <ctime>

#pragma pack(push, 1)

/**
 * @brief 定长二进制记录结构体 —— 随机文件读写的基础单元
 *
 * 每个记录固定为 64 字节，通过 sizeof(FocusRecord) 即可精确计算
 * 文件偏移量，配合 seekg / seekp 实现随机访问。
 *
 * 设计说明（课程要求：Random file processing）：
 * - 必须使用 #pragma pack(1) 禁止字节对齐，确保所有平台下 sizeof 一致
 * - 预留 reserve 字段以便未来扩展字段而不破坏旧文件兼容性
 */
struct FocusRecord {
    int32_t     recordId;           // 记录唯一标识（自增 ID）
    char        plantType[32];      // 种植的植物类型（如 "OakTree"）
    int32_t     durationSeconds;    // 计划专注时长（秒）
    int32_t     actualSeconds;      // 实际专注时长（秒）
    int64_t     timestamp;          // 创建时间戳（Unix 纪元秒）
    uint8_t     isSuccess;          // 是否成功：1=成功, 0=枯萎/失败
    char        reserve[15];        // 预留字段，补齐至 64 字节

    /// 默认构造函数：零初始化
    FocusRecord()
        : recordId(0)
        , durationSeconds(0)
        , actualSeconds(0)
        , timestamp(0)
        , isSuccess(0) {
        std::memset(plantType, 0, sizeof(plantType));
        std::memset(reserve, 0, sizeof(reserve));
    }

    /// 便捷构造函数
    FocusRecord(int32_t id, const char* type, int32_t dur, int32_t actual,
                int64_t ts, bool success)
        : recordId(id)
        , durationSeconds(dur)
        , actualSeconds(actual)
        , timestamp(ts)
        , isSuccess(success ? 1 : 0) {
        std::strncpy(plantType, type, sizeof(plantType) - 1);
        plantType[sizeof(plantType) - 1] = '\0';
        std::memset(reserve, 0, sizeof(reserve));
    }
};

static_assert(sizeof(FocusRecord) == 64, "FocusRecord must be exactly 64 bytes");

#pragma pack(pop)

#endif // FOCUSRECORD_H
