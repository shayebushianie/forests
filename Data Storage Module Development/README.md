# Data Storage Module — Forest 专注森林

> **负责人**：成员3（底层数据存储开发）  
> **模块定位**：基于定长二进制文件的持久化存储层，提供专注记录的 CRUD（增删改查）能力。  
> **对应架构**：Model 层 — `DatabaseManager`

---

## 一句话概述

**零外部依赖，纯 C++ 标准库。** 采用定长二进制文件（Fixed-length Binary File）存储专注记录，通过 `seekg` / `seekp` 实现索引级别的 O(1) 随机定位读写，满足课程大纲 "Random file processing" 要求。

---

## 文件结构

```
Data Storage Module Development/
├── DatabaseCommon.h      # FocusRecord 结构体定义 (64 字节定长)
├── DatabaseManager.h     # DatabaseManager 类声明
├── DatabaseManager.cpp   # DatabaseManager 类实现 (CRUD + 统计)
├── main.cpp              # 独立验证程序 (可脱离其他模块编译运行)
├── forest_data.db        # 二进制数据文件 (程序运行后自动生成)
└── README.md             # 本文件
```

---

## 核心设计

### 定长二进制记录

每条专注会话对应一个 `FocusRecord`，磁盘上严格占 **64 字节**：

| 字段 | 类型 | 大小 | 说明 |
|------|------|------|------|
| `recordId` | `int` | 4B | 记录唯一标识（0-based 索引） |
| `plantType` | `char[32]` | 32B | 植物名称（如 `OakTree`、`Rose`） |
| `durationSeconds` | `int` | 4B | 专注时长（秒） |
| `timestamp` | `long long` | 8B | Unix 时间戳 |
| `isSuccess` | `bool` | 1B | 成功 / 失败（枯萎） |
| `reserve` | `char[15]` | 15B | 预留扩展位 |
| **合计** | | **64B** | |

- 使用 `#pragma pack(push, 1)` 消除编译器字节对齐填充
- 编译期 `static_assert` 保证 `sizeof(FocusRecord) == 64`
- 第 N 条记录的磁盘偏移量 = `N × 64`（等价于 `N << 6`）

### O(1) 记录计数

```
记录总数 = 文件总字节数 / sizeof(FocusRecord) = 文件总字节数 / 64
```

无需遍历整个文件即可获知记录数量。

---

## API 说明 — `DatabaseManager`

### 初始化

```cpp
DatabaseManager dbm("forest_data.db");
bool ok = dbm.initialize();  // 文件不存在时自动创建
```

- 两阶段打开策略：先以 `out` 模式创建空文件，再以 `in | out | binary` 打开
- 幂等性：已初始化的数据库重复调用 `initialize()` 安全无害

### 写入（追加）

```cpp
FocusRecord rec = {};
std::strncpy(rec.plantType, "OakTree", sizeof(rec.plantType) - 1);
rec.durationSeconds = 1800;
rec.timestamp = time(nullptr);
rec.isSuccess = true;

bool ok = dbm.writeRecord(rec);  // recordId 自动赋值为当前记录总数
```

- 记录追加到文件末尾，`recordId` 由函数自动赋值
- `reserve` 字段自动清零
- 写入后立即 `flush()` 确保落盘

### 随机读取（seekg 定位）

```cpp
FocusRecord out;
bool ok = dbm.readRecord(1, out);  // 读取第 2 条记录 (Index = 1)
```

- 偏移量 = `1 × 64 = 64`，读指针直接跳到该位置
- 越界返回 `false`，不会崩溃

### 原地更新（seekp 定位覆写）

```cpp
FocusRecord updated = out;
updated.isSuccess = true;  // 修改字段

bool ok = dbm.updateRecord(1, updated);
```

- 不改变文件大小，不移动其他记录，直接覆写旧数据
- 额外校验 `newRecord.recordId == recordId`，防止索引不一致

### 全表读取

```cpp
std::vector<FocusRecord> all = dbm.getAllRecords();
for (auto& rec : all) { /* 处理每条记录 */ }
```

- 从文件头顺序读取至 EOF

### 记录计数

```cpp
int count = dbm.getRecordCount();  // O(1)
```

---

## 编译 & 运行

### MSVC (Visual Studio)

```
cl /EHsc /std:c++17 main.cpp DatabaseManager.cpp /Fe:forest_test.exe
```

### MinGW-w64 / g++

```
g++ -std=c++17 -o forest_test.exe main.cpp DatabaseManager.cpp
```

### 运行

```
forest_test.exe
```

控制台输出每一步验证结果，运行后在同一目录生成 `forest_data.db` 二进制文件。

---

## 验证程序覆盖范围

`main.cpp` 已包含以下测试场景，可直接编译运行：

| 步骤 | 测试内容 |
|------|---------|
| 第 1 步 | 数据库初始化（文件不存在时自动创建） |
| 第 2 步 | 追加写入 3 条不同类型的记录 |
| 第 3 步 | 随机读取 Index=1 并验证内容一致性 |
| 第 4 步 | 原地更新（翻转 `isSuccess` 字段） |
| 第 5 步 | 重新读取验证原位置数据已被覆写 |
| 第 6 步 | `getAllRecords()` 全表顺序读取 |
| 第 7 步 | 边界条件：越界读取/更新、负索引、recordId 不匹配、重复初始化 |

---

## 集成指南（供其他成员）

### 成员1（UI / 森林画布）

```cpp
#include "DatabaseManager.h"

DatabaseManager dbm("forest_data.db");
dbm.initialize();

// 获取所有历史记录 → 渲染森林
std::vector<FocusRecord> all = dbm.getAllRecords();
for (auto& rec : all) {
    // rec.plantType  → 决定植物类型
    // rec.isSuccess  → 决定健康/枯萎状态
}
```

### 成员2（业务逻辑 / FocusController）

```cpp
// 专注结束时，写入一条记录
FocusRecord rec = {};
strncpy(rec.plantType, session.getPlantName().c_str(), 31);
rec.durationSeconds = session.getDuration();
rec.timestamp = time(nullptr);
rec.isSuccess = session.isSuccess();
dbm.writeRecord(rec);
```

### 成员5（测试工程师）

- 本模块可通过 `DatabaseManager.h` + `DatabaseManager.cpp` 直接链接到 `Qt Test` 或 `Google Test` 项目
- 关键测试点已覆盖：边界读写、越界保护、更新一致性、空文件处理
- 建议额外覆盖：并发场景（多线程同时读写）、文件权限不足时的优雅降级

---

## 注意事项

1. **非线程安全**：`DatabaseManager` 未加锁，多线程访问需调用方自行同步
2. **文件路径**：传入构造函数的路径若含不存在的目录，`initialize()` 会失败
3. **不可删除记录**：当前版本不支持记录删除（会破坏定长偏移量一致性）。如需删除，建议在 `reserve` 或新增字段中标记 `isDeleted`
4. **64 字节硬约束**：切勿增删或调整 `FocusRecord` 字段顺序，否则已有 `.db` 文件将无法正确读取
