#ifndef ABSTRACTPLANT_H
#define ABSTRACTPLANT_H

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

/**
 * @brief 植物抽象基类
 * @details 定义所有植物的通用接口，使用多态实现不同植物的生长逻辑
 * 
 * 继承关系：
 *   AbstractPlant (抽象基类)
 *       ├── Tree (树类)
 *       │       ├── OakTree
 *       │       └── PineTree
 *       └── Flower (花类)
 *               ├── Rose
 *               └── Sunflower
 */
class AbstractPlant
{
public:
    AbstractPlant();
    virtual ~AbstractPlant();

    // ========== 纯虚函数（子类必须实现） ==========
    
    virtual void grow(int minutes) = 0;           ///< 植物生长
    virtual void wither() = 0;                    ///< 植物枯萎
    virtual QString getImagePath() const = 0;     ///< 获取当前阶段图片路径
    virtual QString getTypeName() const = 0;      ///< 获取类型名（用于数据库存储）
    virtual QString getDisplayName() const = 0;   ///< 获取显示名称
    virtual int getProgress() const = 0;          ///< 获取生长进度 0-100
    virtual QString getStageDescription() const = 0; ///< 获取阶段描述

    // ========== 虚函数（子类可重写） ==========
    
    virtual QByteArray serialize() const;         ///< 序列化（供成员3存储）
    virtual void deserialize(const QByteArray& data); ///< 反序列化
    virtual void reset();                         ///< 重置植物状态

    // ========== Getter ==========
    int getTotalMinutes() const { return m_totalMinutes; }
    bool isWithered() const { return m_isWithered; }

protected:
    int m_totalMinutes;      ///< 累计专注分钟数
    bool m_isWithered;       ///< 是否因作弊枯萎
    int m_growThresholds[3]; ///< 生长阶段阈值 [幼苗, 成长, 成熟]
};

#endif // ABSTRACTPLANT_H