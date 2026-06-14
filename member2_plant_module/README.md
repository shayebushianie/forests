# 成员2 - 植物系统模块

## 项目结构
member2_plant_module/
├── include/
│ ├── plant/ # 植物继承体系
│ │ ├── AbstractPlant.h # 抽象基类（第一层）
│ │ ├── Tree.h # 树类（第二层）
│ │ ├── Flower.h # 花类（第二层）
│ │ ├── OakTree.h # 橡树（第三层）
│ │ ├── PineTree.h # 松树（第三层）
│ │ ├── Rose.h # 玫瑰（第三层）
│ │ ├── Sunflower.h # 向日葵（第三层）
│ │ └── PlantFactory.h # 植物工厂
│ ├── controller/
│ │ └── FocusController.h # 专注控制器（状态机）
│ └── logic/ # 业务逻辑层
│ ├── CoinManager.h # 金币管理器
│ ├── AchievementEngine.h # 成就引擎
│ └── StatisticsCalculator.h # 统计计算器
├── src/
│ ├── plant/ # 植物实现
│ ├── controller/ # 控制器实现
│ └── logic/ # 业务逻辑实现
├── forest_plant.pro # Qt 项目文件
├── main.cpp # 测试入口
└── README.md


## 核心功能

### 1. 植物继承体系（多态）

| 层级 | 类名 | 说明 |
|------|------|------|
| 第一层 | `AbstractPlant` | 抽象基类，定义纯虚接口 |
| 第二层 | `Tree` / `Flower` | 树的生长周期更长，花更快成熟 |
| 第三层 | `OakTree` / `PineTree` / `Rose` / `Sunflower` | 具体植物，实现图片路径和阶段描述 |

### 2. 专注控制器（状态机）
IDLE → FOCUSING → SUCCESS（成功，植物生长）
↘ FAILED（失败/作弊，植物枯萎）


### 3. 金币系统

| 规则 | 说明 |
|------|------|
| 基础奖励 | 专注 1 分钟 = 1 金币 |
| 温和模式 | 切屏但未枯萎 → 金币折半 |
| 连续奖励 | 每连续 3 天专注 → 金币 ×1.2（上限 2.0） |
| 持久化 | 自动保存到 `user_data/coins.dat` |

### 4. 成就系统（5项）

| ID | 名称 | 解锁条件 |
|:--:|------|----------|
| 0 | 初试锋芒 | 完成第 1 次专注 |
| 1 | 持之以恒 | 连续专注 7 天 |
| 2 | 植物学家 | 解锁所有 4 种植物 |
| 3 | 专注大师 | 总专注时长达到 100 小时 |
| 4 | 金币富翁 | 累计获得 1000 金币 |

### 5. 标签统计

| 功能 | 说明 |
|------|------|
| 按植物类型统计 | 每种植物的专注时长、会话次数、成功次数 |
| 总统计 | 总专注时长、总成功次数 |
| 持久化 | 自动保存到 `user_data/statistics.dat` |

---

## 接口说明

### 供成员1（UI）调用

```cpp
// 创建植物
AbstractPlant* plant = PlantFactory::createPlant("OakTree");

// 开始专注
FocusController controller;
controller.startFocus(minutes, plant);

// 连接信号
connect(&controller, &FocusController::tick, this, &UI::updateTimer);
connect(&controller, &FocusController::plantUpdated, this, &UI::updatePlant);
connect(&controller, &FocusController::stateChanged, this, &UI::onStateChange);

// 获取金币余额
int balance = CoinManager::getInstance().getBalance();

// 获取成就列表
auto achievements = AchievementEngine::getInstance().getAllAchievements();

// 获取统计数据
int totalMinutes = StatisticsCalculator::getInstance().getTotalMinutes();

### 供成员3（数据存储）调用

// 专注成功后保存记录
FocusRecord record;
strcpy(record.plantType, plant->getTypeName().toStdString().c_str());
record.durationSeconds = minutes * 60;
record.isSuccess = (state == FocusController::SUCCESS);
// timestamp 由成员3填充
dbManager.writeRecord(record);

###供成员4（反作弊）调用

// 检测到作弊时
controller.onCheatDetected();  // 触发植物枯萎，状态变为 FAILED

## 可用植物类型

| 类型名 | 显示名 | 分类 | 成熟时间 |
|--------|--------|------|----------|
| OakTree | 橡树 | 树 | 360 分钟 |
| PineTree | 松树 | 树 | 360 分钟 |
| Rose | 玫瑰 | 花 | 120 分钟 |
| Sunflower | 向日葵 | 花 | 120 分钟 |

## 生长阶段

### 树类（Tree）
| 阶段 | 所需时间 | 进度 |
|------|----------|------|
| 种子 | 0-60 分钟 | 10% |
| 幼苗 | 60-180 分钟 | 33% |
| 成长 | 180-360 分钟 | 66% |
| 成熟 | 360+ 分钟 | 100% |

### 花类（Flower）
| 阶段 | 所需时间 | 进度 |
|------|----------|------|
| 种子 | 0-20 分钟 | 5% |
| 幼苗 | 20-60 分钟 | 30% |
| 成长 | 60-120 分钟 | 60% |
| 成熟 | 120+ 分钟 | 100% |

信号说明（供成员1 UI 使用）
信号	参数	触发时机
tick(int seconds)	剩余秒数	每秒一次
plantUpdated(path, progress, stage)	图片路径、进度、阶段	植物生长/枯萎后
stateChanged(State)	新状态	状态变化时

## 编译运行

### 环境要求
- Qt 6.5.3 + MinGW 11.2.0
- C++17

### 编译步骤

cd member2_plant_module
qmake
mingw32-make
./release/forest_plant.exe

数据文件说明
文件路径	说明
user_data/coins.dat	金币余额
user_data/achievements.dat	成就进度
user_data/statistics.dat	标签统计数据
首次运行时会自动创建 user_data/ 目录和相应文件。

版本记录
版本	日期	说明
v1.0	2026-05-25	植物继承体系 + 工厂模式
v1.1	2026-06-07	添加 FocusController 专注控制器
v1.2	2026-06-14	添加金币系统、成就系统、标签统计