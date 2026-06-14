#include "logic/AchievementEngine.h"
#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QDate>
#include <QDir>

AchievementEngine& AchievementEngine::getInstance()
{
    static AchievementEngine instance;
    return instance;
}

AchievementEngine::AchievementEngine()
    : m_totalFocusMinutes(0)
    , m_totalEarnedCoins(0)
    , m_consecutiveDays(0)
    , m_lastFocusDate(0)
{
    m_dataPath = QCoreApplication::applicationDirPath() + "/user_data/achievements.dat";

    // 初始化成就列表
    m_achievements = {
        {0, "初试锋芒", "完成第1次专注", 1, 0, false},
        {1, "持之以恒", "连续专注7天", 7, 0, false},
        {2, "植物学家", "解锁所有植物类型", 4, 0, false},
        {3, "专注大师", "总专注时长达到100小时", 6000, 0, false},  // 6000分钟 = 100小时
        {4, "金币富翁", "累计获得1000金币", 1000, 0, false}
    };

    loadFromFile();
}

AchievementEngine::~AchievementEngine()
{
    saveToFile();
}

void AchievementEngine::onFocusCompleted(int minutes, int earnedCoins, const QString& plantType)
{
    // 避免 unused parameter 警告
    Q_UNUSED(plantType);  // 添加这行
    
    // 更新总专注时长
    m_totalFocusMinutes += minutes;
    updateProgress(3, minutes);  // 专注大师

    // 更新总获得金币
    m_totalEarnedCoins += earnedCoins;
    updateProgress(4, earnedCoins);  // 金币富翁

    // 更新连续专注天数 - 修正逻辑
    int today = QDate::currentDate().toJulianDay();
    if (m_lastFocusDate == 0) {
        // 第一次专注，连续天数设为1
        m_consecutiveDays = 1;
        updateProgress(1, 1);
    } else if (today == m_lastFocusDate + 1) {
        // 连续天数增加
        m_consecutiveDays++;
        updateProgress(1, 1);  // 每次增加1天时更新
    } else if (today > m_lastFocusDate + 1) {
        // 中断重置
        m_consecutiveDays = 1;
        // 注意：重置时不要调用 updateProgress，因为这是倒退
    }
    m_lastFocusDate = today;

    // 第一次专注
    if (m_totalFocusMinutes >= 1) {
        updateProgress(0, 1);
    }
}

void AchievementEngine::onPlantUnlocked(const QString& plantType)
{
    // 避免重复添加
    if (!plantType.isEmpty() && !m_unlockedPlants.contains(plantType)) {
        m_unlockedPlants.append(plantType);
    }

    // 更新植物学家成就进度
    int unlockedCount = m_unlockedPlants.size();
    if (!m_achievements[2].isUnlocked && unlockedCount >= m_achievements[2].targetValue) {
        m_achievements[2].isUnlocked = true;
        m_achievements[2].currentValue = m_achievements[2].targetValue;
        emit achievementUnlocked(2, m_achievements[2].name);
    } else if (unlockedCount < m_achievements[2].targetValue) {
        m_achievements[2].currentValue = unlockedCount;
        emit progressUpdated(2, unlockedCount, m_achievements[2].targetValue);
    }
}

void AchievementEngine::updateProgress(int id, int delta)
{
    if (id < 0 || id >= m_achievements.size()) return;
    if (m_achievements[id].isUnlocked) return;

    m_achievements[id].currentValue += delta;
    if (m_achievements[id].currentValue > m_achievements[id].targetValue) {
        m_achievements[id].currentValue = m_achievements[id].targetValue;
    }

    emit progressUpdated(id, m_achievements[id].currentValue, m_achievements[id].targetValue);

    if (m_achievements[id].currentValue >= m_achievements[id].targetValue) {
        m_achievements[id].isUnlocked = true;
        emit achievementUnlocked(id, m_achievements[id].name);
        qDebug() << "[Achievement] 解锁成就:" << m_achievements[id].name;
    }
}

void AchievementEngine::checkAndUnlock(int id, int newValue)
{
    if (id < 0 || id >= m_achievements.size()) return;
    if (m_achievements[id].isUnlocked) return;

    if (newValue >= m_achievements[id].targetValue) {
        m_achievements[id].isUnlocked = true;
        m_achievements[id].currentValue = m_achievements[id].targetValue;
        emit achievementUnlocked(id, m_achievements[id].name);
    } else {
        m_achievements[id].currentValue = newValue;
        emit progressUpdated(id, newValue, m_achievements[id].targetValue);
    }
}

QVector<AchievementEngine::Achievement> AchievementEngine::getAllAchievements() const
{
    return m_achievements;
}

int AchievementEngine::getUnlockedCount() const
{
    int count = 0;
    for (const auto& ach : m_achievements) {
        if (ach.isUnlocked) count++;
    }
    return count;
}

void AchievementEngine::saveToFile()
{
    QFile file(m_dataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        // 创建目录
        QDir dir(QCoreApplication::applicationDirPath() + "/user_data");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        if (!file.open(QIODevice::WriteOnly)) return;
    }

    QDataStream stream(&file);
    stream << m_totalFocusMinutes
           << m_totalEarnedCoins
           << m_consecutiveDays
           << m_lastFocusDate
           << m_unlockedPlants;
    file.close();
}

void AchievementEngine::loadFromFile()
{
    QFile file(m_dataPath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QDataStream stream(&file);
    stream >> m_totalFocusMinutes
           >> m_totalEarnedCoins
           >> m_consecutiveDays
           >> m_lastFocusDate
           >> m_unlockedPlants;
    file.close();

    // 重新计算所有成就进度
    // 首先重置成就状态（保留目标值）
    for (auto& ach : m_achievements) {
        ach.currentValue = 0;
        ach.isUnlocked = false;
    }
    
    // 重新计算进度
    updateProgress(0, m_totalFocusMinutes > 0 ? 1 : 0);
    updateProgress(1, m_consecutiveDays);
    updateProgress(3, m_totalFocusMinutes);
    updateProgress(4, m_totalEarnedCoins);
    
    // 重新计算植物学家成就
    onPlantUnlocked("");  // 触发重新计算，参数会被忽略但会遍历已解锁植物
}