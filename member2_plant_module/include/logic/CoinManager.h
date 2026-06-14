#ifndef COINMANAGER_H
#define COINMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief 金币管理器
 * @details 管理用户金币余额，专注成功时增加金币，商城消费时扣除金币
 * 
 * 计算公式：专注1分钟 = 1金币（基础）
 * 温和模式（检测到违规但未枯萎）：金币折半
 * 连续专注奖励：每连续3天专注，金币 ×1.2（上限2.0）
 */
class CoinManager : public QObject
{
    Q_OBJECT

public:
    static CoinManager& getInstance();

    // 获取当前金币余额
    int getBalance() const { return m_balance; }

    // 专注成功时增加金币
    void addCoins(int minutes, bool isGentleMode = false, int consecutiveDays = 0);

    // 商城消费
    bool spendCoins(int amount, const QString& itemName);

    // 保存到文件
    void saveToFile();

    // 从文件加载
    void loadFromFile();

signals:
    void balanceChanged(int newBalance);
    void coinEarned(int earned, int total);
    void coinSpent(int spent, int remaining);

private:
    CoinManager();
    ~CoinManager();

    int calculateEarned(int minutes, bool isGentleMode, int consecutiveDays) const;

    int m_balance;
    QString m_dataPath;
};

#endif // COINMANAGER_H