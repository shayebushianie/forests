#ifndef GACHAMANAGER_H
#define GACHAMANAGER_H

#include <QString>
#include <QVector>
#include <QMap>

class ResinManager;
class CoinManager;

class GachaManager {
public:
    struct VariantDef {
        QString id;           // "golden_oak"
        QString basePlant;    // "OakTree"
        QString displayName;  // "✨ 金色橡树"
        QString icon;         // display emoji
        bool    unlocked;     // 是否已获取
    };

    struct PullResult {
        QString variantId;
        QString displayName;
        QString icon;
        bool    isNew;
        int     resinAwarded; // 重复时返还树脂数
    };

    explicit GachaManager(const QString& filePath);
    ~GachaManager();

    bool load();
    bool save();

    /// 获取所有异色定义（含解锁状态）
    const QVector<VariantDef>& allVariants() const { return m_variants; }

    /// 获取已解锁的异色 ID 列表
    QStringList unlockedVariantIds() const;

    /// 获取异色总数
    int variantCount() const { return m_variants.size(); }

    /// 获取已解锁异色数量
    int unlockedCount() const;

    /// 执行一次抽卡（消耗金币 + 返回结果）
    PullResult pull(CoinManager& coins, ResinManager& resins);

    /// 查询某异色是否已解锁
    bool isUnlocked(const QString& variantId) const;

    /// 直接解锁指定异色（用于树脂兑换）
    bool unlockVariant(const QString& variantId);

    /// 单次抽卡花费
    static int pullCost() { return 100; }

    /// 重复返还树脂
    static int duplicateResin() { return 50; }

private:
    int indexOf(const QString& variantId) const;

    QString m_filePath;
    QVector<VariantDef> m_variants;
};

#endif
