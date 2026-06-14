#ifndef ACHIEVEMENTENGINE_H
#define ACHIEVEMENTENGINE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <functional>

/**
 * @brief 成就系统引擎
 * @details 使用位图算法追踪成就进度，专注成功时触发成就检测
 * 
 * 成就列表：
 *   0: 初试锋芒 - 完成第1次专注
 *   1: 持之以恒 - 连续专注7天
 *   2: 植物学家 - 解锁所有植物类型
 *   3: 专注大师 - 总专注时长达到100小时
 *   4: 金币富翁 - 累计获得1000金币
 */
class AchievementEngine : public QObject
{
    Q_OBJECT

public:
    struct Achievement {
        int id;
        QString name;
        QString description;
        int targetValue;
        int currentValue;
        bool isUnlocked;
    };

    static AchievementEngine& getInstance();

    // 专注成功时调用
    void onFocusCompleted(int minutes, int earnedCoins, const QString& plantType);

    // 解锁植物时调用（商城购买）
    void onPlantUnlocked(const QString& plantType);

    // 获取所有成就
    QVector<Achievement> getAllAchievements() const;

    // 获取已解锁成就数量
    int getUnlockedCount() const;

signals:
    void achievementUnlocked(int id, const QString& name);
    void progressUpdated(int id, int current, int target);

private:
    AchievementEngine();
    ~AchievementEngine();

    void checkAndUnlock(int id, int newValue);
    void saveToFile();
    void loadFromFile();
    void updateProgress(int id, int delta);

    QVector<Achievement> m_achievements;
    QString m_dataPath;
    int m_totalFocusMinutes;
    int m_totalEarnedCoins;
    int m_consecutiveDays;
    int m_lastFocusDate;  // 上次专注的日期（用于计算连续）
    QVector<QString> m_unlockedPlants;
};

#endif // ACHIEVEMENTENGINE_H