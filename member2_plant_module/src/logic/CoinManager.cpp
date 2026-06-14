#include "logic/CoinManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <cmath>
#include <QDir>

CoinManager& CoinManager::getInstance()
{
    static CoinManager instance;
    return instance;
}

CoinManager::CoinManager()
    : m_balance(0)
{
    m_dataPath = QCoreApplication::applicationDirPath() + "/user_data/coins.dat";
    loadFromFile();
}

CoinManager::~CoinManager()
{
    saveToFile();
}

int CoinManager::calculateEarned(int minutes, bool isGentleMode, int consecutiveDays) const
{
    // 基础：1分钟 = 1金币
    int earned = minutes;

    // 温和模式：金币折半
    if (isGentleMode) {
        earned = earned / 2;
    }

    // 连续专注奖励（每连续3天 +20%，上限 +100%）
    if (consecutiveDays >= 3) {
        int bonusLevel = std::min(consecutiveDays / 3, 5); // 最多5级，即15天
        double multiplier = 1.0 + bonusLevel * 0.2;
        if (multiplier > 2.0) multiplier = 2.0;
        earned = static_cast<int>(earned * multiplier);
    }

    return earned;
}

void CoinManager::addCoins(int minutes, bool isGentleMode, int consecutiveDays)
{
    int earned = calculateEarned(minutes, isGentleMode, consecutiveDays);
    if (earned <= 0) return;

    m_balance += earned;
    saveToFile();

    emit coinEarned(earned, m_balance);
    emit balanceChanged(m_balance);

    qDebug() << "[CoinManager] 获得" << earned << "金币，余额:" << m_balance;
}

bool CoinManager::spendCoins(int amount, const QString& itemName)
{
    if (amount <= 0) return false;
    if (m_balance < amount) return false;

    m_balance -= amount;
    saveToFile();

    emit coinSpent(amount, m_balance);
    emit balanceChanged(m_balance);

    qDebug() << "[CoinManager] 购买" << itemName << "，花费" << amount << "金币，余额:" << m_balance;
    return true;
}

void CoinManager::saveToFile()
{
    QFile file(m_dataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        QDir dir(QCoreApplication::applicationDirPath() + "/user_data");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "[CoinManager] 无法保存:" << m_dataPath;
            return;
        }
    }

    QDataStream stream(&file);
    stream << m_balance;
    file.close();
}

void CoinManager::loadFromFile()
{
    QFile file(m_dataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[CoinManager] 无已有数据，初始余额为0";
        return;
    }

    QDataStream stream(&file);
    stream >> m_balance;
    file.close();

    qDebug() << "[CoinManager] 加载余额:" << m_balance;
}