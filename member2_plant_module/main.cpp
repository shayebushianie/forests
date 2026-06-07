#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "include/plant/PlantFactory.h"
#include "include/plant/AbstractPlant.h"
#include "include/controller/FocusController.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== 专注控制器测试 ===";

    // 创建植物
    AbstractPlant* oak = PlantFactory::createPlant("OakTree");
    if (!oak) {
        qDebug() << "创建植物失败";
        return 1;
    }

    qDebug() << "初始状态：" << oak->getDisplayName()
             << "进度:" << oak->getProgress() << "%"
             << "阶段:" << oak->getStageDescription();

    // 创建控制器
    FocusController controller;

    // 连接信号（模拟 UI 响应）
    QObject::connect(&controller, &FocusController::tick,
        [](int seconds) {
            if (seconds % 10 == 0) {  // 每10秒打印一次
                qDebug() << "倒计时:" << seconds << "秒";
            }
        });

    QObject::connect(&controller, &FocusController::plantUpdated,
        [](const QString& path, int progress, const QString& stage) {
            qDebug() << "植物更新 - 进度:" << progress << "%"
                     << "阶段:" << stage
                     << "图片:" << path;
        });

    QObject::connect(&controller, &FocusController::stateChanged,
        [](FocusController::State state) {
            QString stateName;
            switch(state) {
                case FocusController::IDLE: stateName = "空闲"; break;
                case FocusController::FOCUSING: stateName = "专注中"; break;
                case FocusController::SUCCESS: stateName = "成功"; break;
                case FocusController::FAILED: stateName = "失败"; break;
                default: stateName = "未知"; break;
            }
            qDebug() << "状态变更:" << stateName;
        });

    // 开始专注：2分钟（方便测试）
    qDebug() << "\n开始专注 2 分钟...";
    controller.startFocus(2, oak);

    // 运行 3 秒后模拟作弊（测试用）
    QTimer::singleShot(3000, &controller, &FocusController::onCheatDetected);

    // 运行事件循环
    // QTimer::singleShot(5000, &app, &QCoreApplication::quit);
    return app.exec();
}