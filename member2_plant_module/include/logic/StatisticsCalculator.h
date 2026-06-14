#ifndef STATISTICSCALCULATOR_H
#define STATISTICSCALCULATOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>

/**
 * @brief 标签统计计算器
 * @details 按植物类型（标签）统计专注时长，用于UI展示饼图
 */
class StatisticsCalculator : public QObject
{
    Q_OBJECT

public:
    struct TagStat {
        QString tagName;      // 标签名（植物类型）
        int totalMinutes;     // 总专注分钟
        int sessionCount;     // 会话次数
        int successCount;     // 成功次数
    };

    static StatisticsCalculator& getInstance();

    // 专注成功时调用
    void addRecord(const QString& plantType, int minutes, bool isSuccess);

    // 获取所有标签统计
    QVector<TagStat> getAllStats() const;

    // 获取总专注时长
    int getTotalMinutes() const;

    // 获取总成功次数
    int getTotalSuccessCount() const;

    // 按标签筛选统计
    TagStat getStatByTag(const QString& plantType) const;

    // 清空所有统计（用于测试）
    void clearAll();

signals:
    void statisticsUpdated();

private:
    StatisticsCalculator();
    ~StatisticsCalculator();

    void saveToFile();
    void loadFromFile();

    QMap<QString, TagStat> m_stats;
    QString m_dataPath;
    int m_totalMinutes;
    int m_totalSuccessCount;
};

#endif // STATISTICSCALCULATOR_H