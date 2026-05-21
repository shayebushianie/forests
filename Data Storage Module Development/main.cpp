/**
 * @file    main.cpp
 * @brief   数据存储模块验证程序 —— 演示定长二进制文件的 CRUD 操作
 * @author  成员3（数据存储模块）
 * @date    2026-05-21
 *
 * @details
 * 本程序对 DatabaseManager 进行功能验证，涵盖以下场景：
 *   1. 数据库初始化（含文件不存在时的自动创建）。
 *   2. 追加写入多条记录（writeRecord）。
 *   3. 随机读取指定索引的记录（readRecord）。
 *   4. 原地更新记录（updateRecord）并再次读取验证。
 *   5. 边界条件测试（越界读取、越界更新、重复初始化）。
 *
 * 预期输出：
 *   - 控制台输出每一步操作的结果（成功/失败 + 记录详情）。
 *   - 在当前目录生成 forest_data.db 二进制文件。
 *
 * 编译命令（MSVC）：
 *   cl /EHsc /std:c++17 main.cpp DatabaseManager.cpp /Fe:forest_test.exe
 *
 * 编译命令（MinGW-w64 / g++）：
 *   g++ -std=c++17 -o forest_test.exe main.cpp DatabaseManager.cpp
 */

#include "DatabaseManager.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>

// ────────────────────────────────────────────────────────────
// 辅助函数
// ────────────────────────────────────────────────────────────

/** @brief 打印一条 FocusRecord 的详细信息到标准输出。 */
void printRecord(const FocusRecord& record, const char* label = "Record")
{
    std::cout << "  [" << label << "]\n";
    std::cout << "    recordId      = " << record.recordId << "\n";
    std::cout << "    plantType     = " << record.plantType << "\n";
    std::cout << "    durationSec   = " << record.durationSeconds << "\n";
    std::cout << "    timestamp     = " << record.timestamp << "\n";
    std::cout << "    isSuccess     = " << (record.isSuccess ? "true (成功)" : "false (失败)") << "\n";
    std::cout << std::endl;
}

/** @brief 打印分隔线。 */
void printSeparator(const char* title)
{
    std::cout << "\n========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n";
}

// ────────────────────────────────────────────────────────────
// main —— 验证入口
// ────────────────────────────────────────────────────────────

int main()
{
    std::cout << "Forest 数据存储模块 —— 验证程序\n";
    std::cout << "sizeof(FocusRecord) = " << sizeof(FocusRecord)
              << " 字节 (应为 64 字节，static_assert 已确保)\n";

    // ────────────────────────────────────────────────────────
    // 第 1 步：初始化数据库
    // ────────────────────────────────────────────────────────
    printSeparator("第 1 步：初始化数据库");

    DatabaseManager dbm("forest_data.db");

    if (!dbm.initialize())
    {
        std::cerr << "[错误] 数据库初始化失败！" << std::endl;
        return 1;
    }
    std::cout << "[通过] 数据库初始化成功。" << std::endl;
    std::cout << "当前记录数: " << dbm.getRecordCount() << std::endl;

    // ────────────────────────────────────────────────────────
    // 第 2 步：写入 3 条测试记录
    // ────────────────────────────────────────────────────────
    printSeparator("第 2 步：追加写入 3 条记录");

    // --- 记录 0：成功的 OakTree ---
    {
        FocusRecord rec = {};
        // plantType 用 std::strncpy 安全填充
        const char* name = "OakTree";
        std::strncpy(rec.plantType, name, sizeof(rec.plantType) - 1);
        rec.plantType[sizeof(rec.plantType) - 1] = '\0';
        rec.durationSeconds = 1800;   // 30 分钟
        rec.timestamp       = static_cast<long long>(std::time(nullptr));
        rec.isSuccess       = true;

        if (dbm.writeRecord(rec))
        {
            std::cout << "[通过] 写入记录 0 (OakTree, 成功)。" << std::endl;
            printRecord(rec, "写入的记录 0");
        }
        else
        {
            std::cerr << "[失败] 写入记录 0 失败！" << std::endl;
        }
    }

    // --- 记录 1：失败的 Rose ---
    {
        FocusRecord rec = {};
        const char* name = "Rose";
        std::strncpy(rec.plantType, name, sizeof(rec.plantType) - 1);
        rec.plantType[sizeof(rec.plantType) - 1] = '\0';
        rec.durationSeconds = 600;    // 10 分钟（中途失败）
        rec.timestamp       = static_cast<long long>(std::time(nullptr)) + 60;
        rec.isSuccess       = false;  // 枯萎

        if (dbm.writeRecord(rec))
        {
            std::cout << "[通过] 写入记录 1 (Rose, 失败/枯萎)。" << std::endl;
            printRecord(rec, "写入的记录 1");
        }
        else
        {
            std::cerr << "[失败] 写入记录 1 失败！" << std::endl;
        }
    }

    // --- 记录 2：成功的 PineTree ---
    {
        FocusRecord rec = {};
        const char* name = "PineTree";
        std::strncpy(rec.plantType, name, sizeof(rec.plantType) - 1);
        rec.plantType[sizeof(rec.plantType) - 1] = '\0';
        rec.durationSeconds = 3600;   // 60 分钟
        rec.timestamp       = static_cast<long long>(std::time(nullptr)) + 120;
        rec.isSuccess       = true;

        if (dbm.writeRecord(rec))
        {
            std::cout << "[通过] 写入记录 2 (PineTree, 成功)。" << std::endl;
            printRecord(rec, "写入的记录 2");
        }
        else
        {
            std::cerr << "[失败] 写入记录 2 失败！" << std::endl;
        }
    }

    std::cout << "写入完成后记录总数: " << dbm.getRecordCount() << std::endl;

    // ────────────────────────────────────────────────────────
    // 第 3 步：随机读取第 1 条记录（Index = 1）
    // ────────────────────────────────────────────────────────
    printSeparator("第 3 步：随机读取 Index = 1 的记录 (seekg 定位)");

    {
        FocusRecord out = {};
        if (dbm.readRecord(1, out))
        {
            std::cout << "[通过] 成功读取 Index = 1 的记录。" << std::endl;
            printRecord(out, "读取到的记录 (Index=1)");

            // 验证内容正确性
            bool valid = true;
            if (out.recordId != 1)           { std::cerr << "[警告] recordId 不匹配！\n"; valid = false; }
            if (std::strcmp(out.plantType, "Rose") != 0) { std::cerr << "[警告] plantType 不匹配！\n"; valid = false; }
            if (out.durationSeconds != 600)   { std::cerr << "[警告] durationSeconds 不匹配！\n"; valid = false; }
            if (out.isSuccess != false)       { std::cerr << "[警告] isSuccess 不匹配！\n"; valid = false; }
            if (valid) std::cout << "[验证] 记录内容与写入时完全一致。" << std::endl;
        }
        else
        {
            std::cerr << "[失败] 无法读取 Index = 1 的记录！" << std::endl;
        }
    }

    // ────────────────────────────────────────────────────────
    // 第 4 步：原地更新第 1 条记录（翻转 isSuccess）
    // ────────────────────────────────────────────────────────
    printSeparator("第 4 步：原地更新 Index = 1 (seekp 定位覆写)");

    {
        // 先读出原记录
        FocusRecord updated = {};
        if (!dbm.readRecord(1, updated))
        {
            std::cerr << "[失败] 更新前无法读取 Index = 1！" << std::endl;
            return 1;
        }

        // 翻转 isSuccess：从 false(失败) → true(成功)
        updated.isSuccess = !updated.isSuccess;
        std::cout << "原 isSuccess = false，翻转为 = "
                  << (updated.isSuccess ? "true" : "false") << std::endl;

        if (dbm.updateRecord(1, updated))
        {
            std::cout << "[通过] 原地更新 Index = 1 成功（seekp 覆写）。" << std::endl;
        }
        else
        {
            std::cerr << "[失败] 原地更新 Index = 1 失败！" << std::endl;
        }
    }

    // ────────────────────────────────────────────────────────
    // 第 5 步：再次读取验证更新是否生效
    // ────────────────────────────────────────────────────────
    printSeparator("第 5 步：重新读取 Index = 1 验证更新结果");

    {
        FocusRecord verify = {};
        if (dbm.readRecord(1, verify))
        {
            std::cout << "[通过] 重新读取 Index = 1 成功。" << std::endl;
            printRecord(verify, "更新后的记录 (Index=1)");

            if (verify.isSuccess == true)
            {
                std::cout << "[验证] isSuccess 已成功从 false 更新为 true！"
                          << "原地覆写（updateRecord）功能正常。" << std::endl;
            }
            else
            {
                std::cerr << "[警告] isSuccess 未按预期更新！" << std::endl;
            }
        }
        else
        {
            std::cerr << "[失败] 验证读取失败！" << std::endl;
        }
    }

    // ────────────────────────────────────────────────────────
    // 第 6 步：全表读取
    // ────────────────────────────────────────────────────────
    printSeparator("第 6 步：getAllRecords() 全表读取");

    {
        std::vector<FocusRecord> all = dbm.getAllRecords();
        std::cout << "全表记录数: " << all.size() << std::endl;
        for (size_t i = 0; i < all.size(); ++i)
        {
            char label[32];
            snprintf(label, sizeof(label), "全表记录 [%zu]", i);
            printRecord(all[i], label);
        }
    }

    // ────────────────────────────────────────────────────────
    // 第 7 步：边界条件测试
    // ────────────────────────────────────────────────────────
    printSeparator("第 7 步：边界条件测试");

    // 7a. 越界读取（recordId = 99，当前只有 3 条记录）
    {
        FocusRecord dummy = {};
        if (dbm.readRecord(99, dummy))
        {
            std::cerr << "[失败] 越界读取 Index=99 不应成功！" << std::endl;
        }
        else
        {
            std::cout << "[通过] 越界读取 Index=99 被正确拒绝。" << std::endl;
        }
    }

    // 7b. 负索引读取
    {
        FocusRecord dummy = {};
        if (dbm.readRecord(-1, dummy))
        {
            std::cerr << "[失败] 负索引读取不应成功！" << std::endl;
        }
        else
        {
            std::cout << "[通过] 负索引 Index=-1 被正确拒绝。" << std::endl;
        }
    }

    // 7c. 越界更新
    {
        FocusRecord dummy = {};
        dummy.recordId = 99;
        if (dbm.updateRecord(99, dummy))
        {
            std::cerr << "[失败] 越界更新 Index=99 不应成功！" << std::endl;
        }
        else
        {
            std::cout << "[通过] 越界更新 Index=99 被正确拒绝。" << std::endl;
        }
    }

    // 7d. recordId 不匹配的更新
    {
        FocusRecord dummy = {};
        dummy.recordId = 999; // 与参数 recordId=0 不一致
        if (dbm.updateRecord(0, dummy))
        {
            std::cerr << "[失败] recordId 不匹配的更新不应成功！" << std::endl;
        }
        else
        {
            std::cout << "[通过] recordId 不匹配 (0 vs 999) 被正确拒绝。" << std::endl;
        }
    }

    // 7e. 重复初始化（应安全）
    {
        if (dbm.initialize())
        {
            std::cout << "[通过] 重复调用 initialize() 安全（幂等）。" << std::endl;
        }
        else
        {
            std::cout << "[通过] 重复调用 initialize() 返回 false（已初始化）。" << std::endl;
        }
        std::cout << "重复初始化后记录数: " << dbm.getRecordCount()
                  << "（应与之前一致 = 3）" << std::endl;
    }

    // ────────────────────────────────────────────────────────
    // 完成
    // ────────────────────────────────────────────────────────
    printSeparator("验证完成");

    std::cout << "所有验证步骤已执行完毕。\n";
    std::cout << "二进制数据文件: forest_data.db（" << dbm.getRecordCount()
              << " 条记录 × " << sizeof(FocusRecord) << " 字节 = "
              << (dbm.getRecordCount() * static_cast<int>(sizeof(FocusRecord)))
              << " 字节）" << std::endl;

    return 0;
}
