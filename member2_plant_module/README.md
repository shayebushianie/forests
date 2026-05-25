# 成员2 - 植物系统模块

## 项目结构
forest_plant/
├── include/
│ ├── DatabaseCommon.h # 成员3：数据存储公共定义
│ ├── DatabaseManager.h # 成员3：数据库管理器
│ └── plant/ # 成员2：植物系统
│ ├── AbstractPlant.h # 植物抽象基类
│ ├── Tree.h # 树类（第二层）
│ ├── Flower.h # 花类（第二层）
│ ├── OakTree.h # 橡树（第三层）
│ ├── PineTree.h # 松树（第三层）
│ ├── Rose.h # 玫瑰（第三层）
│ ├── Sunflower.h # 向日葵（第三层）
│ └── PlantFactory.h # 植物工厂
├── src/
│ ├── DatabaseManager.cpp # 成员3：数据库管理器实现
│ └── plant/ # 成员2：植物系统实现
│ ├── AbstractPlant.cpp
│ ├── Tree.cpp
│ ├── Flower.cpp
│ ├── OakTree.cpp
│ ├── PineTree.cpp
│ ├── Rose.cpp
│ ├── Sunflower.cpp
│ └── PlantFactory.cpp
├── forest_plant.pro # Qt 项目文件
├── main.cpp # 测试入口
└── .gitignore


## 接口说明

### 供成员1（UI）调用

| 方法 | 说明 |
|------|------|
| `PlantFactory::createPlant(typeName)` | 创建植物对象 |
| `plant->getImagePath()` | 获取当前阶段图片路径 |
| `plant->getProgress()` | 获取生长进度 (0-100) |
| `plant->getStageDescription()` | 获取阶段描述 |
| `plant->getDisplayName()` | 获取显示名称 |

### 供成员3（数据存储）调用

| 方法 | 说明 |
|------|------|
| `plant->serialize()` | 序列化为二进制数据 |
| `plant->getTypeName()` | 获取类型名（存入 plantType 字段） |

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

待完成
FocusController（专注控制器，番茄钟状态机）

图片资源（由成员1准备）

与成员4（反作弊监控）对接

版本记录
版本	日期	说明
v1.0	2026-05-25	植物继承体系 + 工厂模式完成