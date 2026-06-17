#include "StoreManager.h"
#include "CoinManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>

StoreManager::StoreManager(const QString& filePath)
    : m_filePath(filePath) {

    // 注册所有植物（固定顺序，索引作为存储 key）
    // price <= 0 表示初始解锁
    m_plants = {
        { QStringLiteral("OakTree"),    QStringLiteral("🌳 橡树"),     QStringLiteral("🌳"),    0,    true  },
        { QStringLiteral("Rose"),       QStringLiteral("🌹 玫瑰"),     QStringLiteral("🌹"),  200,   false },
        { QStringLiteral("Sunflower"),  QStringLiteral("🌻 向日葵"),   QStringLiteral("🌻"),  350,   false },
        { QStringLiteral("PineTree"),   QStringLiteral("🌲 松树"),     QStringLiteral("🌲"),  500,   false },
    };
}

StoreManager::~StoreManager() {
    save();
}

bool StoreManager::load() {
    QFile file(m_filePath);
    if (!file.exists()) {
        // 首次运行：仅 OakTree 解锁
        save();
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[StoreManager] 无法打开文件";
        return false;
    }

    QDataStream in(&file);
    quint32 count;
    in >> count;

    for (quint32 i = 0; i < count && i < (quint32)m_plants.size(); ++i) {
        quint8 flag;
        in >> flag;
        m_plants[i].unlocked = (flag != 0);
    }
    file.close();

    qDebug() << "[StoreManager] 加载" << count << "种植物的解锁状态";
    return true;
}

bool StoreManager::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[StoreManager] 无法保存";
        return false;
    }

    QDataStream out(&file);
    out << (quint32)m_plants.size();
    for (const auto& p : m_plants) {
        out << (quint8)(p.unlocked ? 1 : 0);
    }
    file.close();
    return true;
}

QVector<StoreManager::PlantInfo> StoreManager::allPlants() const {
    return m_plants;
}

QStringList StoreManager::unlockedTypeNames() const {
    QStringList list;
    for (const auto& p : m_plants) {
        if (p.unlocked) {
            list << p.typeName;
        }
    }
    return list;
}

bool StoreManager::isUnlocked(const QString& typeName) const {
    int idx = indexOf(typeName);
    return idx >= 0 && m_plants[idx].unlocked;
}

bool StoreManager::unlock(const QString& typeName, CoinManager& coins) {
    int idx = indexOf(typeName);
    if (idx < 0) return false;
    if (m_plants[idx].unlocked) return true;  // 已解锁

    int price = m_plants[idx].price;
    if (price <= 0) {
        // 免费植物直接解锁
        m_plants[idx].unlocked = true;
        save();
        return true;
    }

    if (!coins.spend(price)) {
        qWarning() << "[StoreManager] 金币不足，需要" << price;
        return false;
    }

    m_plants[idx].unlocked = true;
    save();
    qDebug() << "[StoreManager] 解锁:" << typeName << "花费" << price << "金币";
    return true;
}

int StoreManager::priceOf(const QString& typeName) const {
    int idx = indexOf(typeName);
    return idx >= 0 ? m_plants[idx].price : -1;
}

int StoreManager::indexOf(const QString& typeName) const {
    for (int i = 0; i < m_plants.size(); ++i) {
        if (m_plants[i].typeName == typeName) return i;
    }
    return -1;
}
