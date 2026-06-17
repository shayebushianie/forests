#include "CoinManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <algorithm>

CoinManager::CoinManager(const QString& filePath)
    : m_filePath(filePath)
    , m_coins(0)
    , m_totalEarned(0) {}

CoinManager::~CoinManager() {
    save();
}

bool CoinManager::load() {
    QFile file(m_filePath);
    if (!file.exists()) {
        m_coins = 0;
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[CoinManager] 无法打开金币文件";
        return false;
    }
    QDataStream in(&file);
    in >> m_coins;
    if (!in.atEnd()) {
        in >> m_totalEarned;
    }
    file.close();
    qDebug() << "[CoinManager] 文件:" << m_filePath << "加载金币:" << m_coins << "累计赚取:" << m_totalEarned;
    return true;
}

bool CoinManager::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[CoinManager] 无法保存金币文件";
        return false;
    }
    QDataStream out(&file);
    out << m_coins << m_totalEarned;
    file.close();
    return true;
}

int CoinManager::earn(int actualSeconds, bool gentle) {
    // 每 5 分钟 = 1 金币 (300秒)
    int earned = std::max(1, actualSeconds / 300);
    if (gentle) {
        earned = std::max(1, earned / 2);  // 温和模式减半
    }
    m_coins += earned;
    m_totalEarned += earned;
    save();
    qDebug() << "[CoinManager] +" << earned << "金币 (余额:" << m_coins << ")"
             << (gentle ? "[温和折半]" : "");
    return earned;
}

bool CoinManager::spend(int amount) {
    if (amount <= 0 || amount > m_coins) return false;
    m_coins -= amount;
    save();
    return true;
}
