#include <QCoreApplication>
#include <QDebug>
#include "include/logic/CoinManager.h"
#include "include/logic/AchievementEngine.h"
#include "include/logic/StatisticsCalculator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== 金币系统测试 ===";
    CoinManager::getInstance().addCoins(30, false, 0);
    qDebug() << "当前余额:" << CoinManager::getInstance().getBalance();

    qDebug() << "\n=== 成就系统测试 ===";
    AchievementEngine::getInstance().onFocusCompleted(30, 30, "OakTree");
    AchievementEngine::getInstance().onPlantUnlocked("OakTree");
    AchievementEngine::getInstance().onPlantUnlocked("PineTree");
    AchievementEngine::getInstance().onPlantUnlocked("Rose");
    AchievementEngine::getInstance().onPlantUnlocked("Sunflower");

    qDebug() << "\n=== 统计系统测试 ===";
    StatisticsCalculator::getInstance().addRecord("OakTree", 30, true);
    StatisticsCalculator::getInstance().addRecord("Rose", 25, true);
    qDebug() << "总专注时长:" << StatisticsCalculator::getInstance().getTotalMinutes() << "分钟";

    return 0;
}