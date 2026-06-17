#ifndef ABSTRACTPLANT_H
#define ABSTRACTPLANT_H

#include <QString>
#include <QList>

/**
 * @brief 植物的生长阶段枚举
 */
enum class GrowthStage {
    Seed,       ///< 种子（刚种下）
    Sprout,     ///< 发芽
    Growing,    ///< 成长中
    Mature,     ///< 成熟（完全体）
    Withered    ///< 枯萎
};

/**
 * @brief 抽象植物基类（继承体系第一层）
 *
 * 课程要求：5个以上类、2层以上继承。
 * 本类作为抽象基类，定义所有植物的公共接口。
 *
 * 继承体系：
 *   AbstractPlant（第一层，抽象基类）
 *   ├── Tree（第二层，抽象类）
 *   │   ├── OakTree（第三层，具体类）
 *   │   └── PineTree（第三层，具体类）
 *   └── Flower（第二层，抽象类）
 *       ├── Rose（第三层，具体类）
 *       └── Sunflower（第三层，具体类）
 */
class AbstractPlant {
public:
    /**
     * @brief 构造函数
     * @param name 植物名称
     * @param growTime 从种子到成熟所需的总专注时长（秒）
     */
    AbstractPlant(const QString& name, int growTime);
    virtual ~AbstractPlant() = default;

    // ─── 纯虚函数（子类必须实现） ────────────────────────────
    /** @brief 植物生长逻辑（专注一秒调用一次） */
    virtual void grow() = 0;
    /** @brief 植物枯萎逻辑 */
    virtual void wither() = 0;
    /** @brief 获取植物显示用的 Unicode 字符 / Emoji */
    virtual QChar displayChar() const = 0;

    // ─── 公共接口 ────────────────────────────────────────────
    const QString& name() const { return m_name; }
    GrowthStage stage() const { return m_stage; }
    int growTime() const { return m_growTime; }
    int currentGrowth() const { return m_currentGrowth; }
    double progress() const {
        return m_growTime > 0
                   ? static_cast<double>(m_currentGrowth) / m_growTime
                   : 0.0;
    }
    bool isAlive() const {
        return m_stage != GrowthStage::Withered;
    }

    /** @brief 恢复生长进度（用于从失败/取消中恢复） */
    void setGrowth(int seconds);

protected:
    QString     m_name;           ///< 植物名称
    GrowthStage m_stage;          ///< 当前生长阶段
    int         m_growTime;       ///< 总生长所需秒数
    int         m_currentGrowth;  ///< 当前生长进度（秒）
};

#endif // ABSTRACTPLANT_H
