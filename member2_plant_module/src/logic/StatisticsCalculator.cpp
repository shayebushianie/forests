#include "logic/StatisticsCalculator.h"
#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QDir>

StatisticsCalculator& StatisticsCalculator::getInstance()
{
    static StatisticsCalculator instance;
    return instance;
}

StatisticsCalculator::StatisticsCalculator()
    : m_totalMinutes(0)
    , m_totalSuccessCount(0)
{
    m_dataPath = QCoreApplication::applicationDirPath() + "/user_data/statistics.dat";
    loadFromFile();
}

StatisticsCalculator::~StatisticsCalculator()
{
    saveToFile();
}

void StatisticsCalculator::addRecord(const QString& plantType, int minutes, bool isSuccess)
{
    if (minutes <= 0) return;

    // 更新总统计
    m_totalMinutes += minutes;
    if (isSuccess) {
        m_totalSuccessCount++;
    }

    // 更新标签统计
    auto it = m_stats.find(plantType);
    if (it == m_stats.end()) {
        TagStat newStat;
        newStat.tagName = plantType;
        newStat.totalMinutes = minutes;
        newStat.sessionCount = 1;
        newStat.successCount = isSuccess ? 1 : 0;
        m_stats[plantType] = newStat;
    } else {
        it->totalMinutes += minutes;
        it->sessionCount++;
        if (isSuccess) {
            it->successCount++;
        }
    }

    saveToFile();
    emit statisticsUpdated();

    qDebug() << "[Statistics] 记录: 标签=" << plantType << ", 时长=" << minutes << "分钟, 成功=" << isSuccess;
}

QVector<StatisticsCalculator::TagStat> StatisticsCalculator::getAllStats() const
{
    QVector<TagStat> result;
    for (auto it = m_stats.begin(); it != m_stats.end(); ++it) {
        result.append(it.value());
    }
    return result;
}

int StatisticsCalculator::getTotalMinutes() const
{
    return m_totalMinutes;
}

int StatisticsCalculator::getTotalSuccessCount() const
{
    return m_totalSuccessCount;
}

StatisticsCalculator::TagStat StatisticsCalculator::getStatByTag(const QString& plantType) const
{
    auto it = m_stats.find(plantType);
    if (it != m_stats.end()) {
        return it.value();
    }
    TagStat empty;
    empty.tagName = plantType;
    empty.totalMinutes = 0;
    empty.sessionCount = 0;
    empty.successCount = 0;
    return empty;
}

void StatisticsCalculator::clearAll()
{
    m_stats.clear();
    m_totalMinutes = 0;
    m_totalSuccessCount = 0;
    saveToFile();
    emit statisticsUpdated();
}

void StatisticsCalculator::saveToFile()
{
    QFile file(m_dataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        // 尝试创建目录
        QDir dir(QCoreApplication::applicationDirPath() + "/user_data");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "[Statistics] 无法保存:" << m_dataPath;
            return;
        }
    }

    QDataStream stream(&file);
    
    // 先写简单类型
    stream << m_totalMinutes << m_totalSuccessCount;
    
    // 手动序列化 QMap
    stream << m_stats.size();
    for (auto it = m_stats.begin(); it != m_stats.end(); ++it) {
        stream << it.key();                        // QString tagName
        stream << it.value().totalMinutes;
        stream << it.value().sessionCount;
        stream << it.value().successCount;
    }
    
    file.close();
}

void StatisticsCalculator::loadFromFile()
{
    QFile file(m_dataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream stream(&file);
    
    // 读取简单类型
    stream >> m_totalMinutes >> m_totalSuccessCount;
    
    // 手动反序列化 QMap
    int size;
    stream >> size;
    m_stats.clear();
    for (int i = 0; i < size; ++i) {
        QString key;
        TagStat value;
        stream >> key;
        stream >> value.totalMinutes >> value.sessionCount >> value.successCount;
        value.tagName = key;
        m_stats[key] = value;
    }
    
    file.close();
}