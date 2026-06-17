#ifndef COINMANAGER_H
#define COINMANAGER_H

#include <QString>

/**
 * @brief 金币管理器
 *
 * [新增] 专注成功获得金币，温和模式减半。
 * 持久化到单独二进制文件。
 */
class CoinManager {
public:
    explicit CoinManager(const QString& filePath);
    ~CoinManager();

    bool load();
    bool save();

    int balance() const { return m_coins; }
    int totalEarned() const { return m_totalEarned; }

    /**
     * @brief 赚取金币
     * @param actualSeconds 实际专注秒数
     * @param gentle 是否为温和模式（减半）
     * @return 本次赚取的金币数
     */
    int earn(int actualSeconds, bool gentle);

    bool spend(int amount);

    // TEST CODE - REMOVE AFTER TESTING
    void debugSetBalance(int amount) { m_coins = amount; m_totalEarned = amount; save(); }

private:
    QString m_filePath;
    int     m_coins;
    int     m_totalEarned = 0;
};

#endif // COINMANAGER_H
