#include <QApplication>
#include <QDebug>
#include <QTimer>
#include "include/plant/PlantFactory.h"
#include "include/plant/AbstractPlant.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== 植物系统测试 ===";
    
    // 测试工厂模式创建植物
    AbstractPlant* oak = PlantFactory::createPlant("OakTree");
    if (oak) {
        qDebug() << "创建植物:" << oak->getDisplayName();
        qDebug() << "类型名:" << oak->getTypeName();
        qDebug() << "初始进度:" << oak->getProgress() << "%";
        qDebug() << "初始阶段:" << oak->getStageDescription();
        
        // 模拟专注 90 分钟
        oak->grow(90);
        qDebug() << "专注90分钟后 - 进度:" << oak->getProgress() << "%";
        qDebug() << "阶段:" << oak->getStageDescription();
        qDebug() << "图片路径:" << oak->getImagePath();
        
        delete oak;
    }
    
    QTimer::singleShot(5000, &app, &QApplication::quit);
    return app.exec();
}