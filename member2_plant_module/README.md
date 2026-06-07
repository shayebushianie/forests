# 成员2 - 植物系统模块

## 项目结构
member2_plant_module/
├── include/
│ ├── plant/ # 植物继承体系
│ │ ├── AbstractPlant.h # 抽象基类
│ │ ├── Tree.h # 树类（第二层）
│ │ ├── Flower.h # 花类（第二层）
│ │ ├── OakTree.h # 橡树
│ │ ├── PineTree.h # 松树
│ │ ├── Rose.h # 玫瑰
│ │ ├── Sunflower.h # 向日葵
│ │ └── PlantFactory.h # 植物工厂
│ └── controller/
│ └── FocusController.h # 专注控制器
├── src/
│ ├── plant/ # 植物实现
│ └── controller/
│ └── FocusController.cpp # 控制器实现
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



## 接口说明

### 供成员1（UI）调用

```cpp
// 创建植物
AbstractPlant* plant = PlantFactory::createPlant("OakTree");

// 开始专注
FocusController controller;
controller.startFocus(minutes, plant);

// 连接信号（接收状态更新）
connect(&controller, &FocusController::tick, this, &UI::updateTimer);
connect(&controller, &FocusController::plantUpdated, this, &UI::updatePlantImage);
connect(&controller, &FocusController::stateChanged, this, &UI::onStateChanged);|

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

```bash
# 进入项目目录
cd forest_plant

# 生成 Makefile
qmake

# 编译
mingw32-make

# 运行
./release/forest_plant.exe

版本记录
版本	日期	说明
v1.0	2026-05-25	植物继承体系 + 工厂模式
v1.1	2026-06-07	添加 FocusController 专注控制器