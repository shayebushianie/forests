#ifndef STOREMANAGER_H
#define STOREMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>

class CoinManager;

/**
 * @brief 商店管理器
 *
 * [新增] 管理植物种子的解锁状态，持久化到二进制文件。
 * 初始仅 OakTree 可用，其余植物需用金币购买。
 */
class StoreManager {
public:
    struct PlantInfo {
        QString typeName;     // 内部类型名 "OakTree"
        QString displayName;  // 显示名 "🌳 橡树"
        QString icon;         // Emoji 图标
        int     price;        // 价格（金币）
        bool    unlocked;     // 当前是否已解锁
    };

    explicit StoreManager(const QString& filePath);
    ~StoreManager();

    bool load();
    bool save();

    /// 获取所有植物信息（包含解锁状态）
    QVector<PlantInfo> allPlants() const;

    /// 获取已解锁的植物类型名列表
    QStringList unlockedTypeNames() const;

    /// 查询某植物是否已解锁
    bool isUnlocked(const QString& typeName) const;

    /// 购买解锁（扣除金币），返回是否成功
    bool unlock(const QString& typeName, CoinManager& coins);

    /// 查询某植物的价格
    int priceOf(const QString& typeName) const;

private:
    int indexOf(const QString& typeName) const;

    QString m_filePath;
    QVector<PlantInfo> m_plants;  // 固定顺序：OakTree, PineTree, Rose, Sunflower, ...
};

#endif // STOREMANAGER_H
