/**
 * @file main.cpp
 * @brief Focus Forest 专注森林 —— 程序入口
 *
 * 课程项目：华南理工大学 Windows C++ 桌面端专注软件开发
 * 技术栈：Qt5/Qt6 (QWidget + QSS) + Windows API
 *
 * 架构模式：MVC
 *   - Model:      FocusSession, AbstractPlant 体系, FocusRecord
 *   - View:       MainWindow, GardenCanvas
 *   - Controller: FocusController
 *   - Monitor:    SystemMonitor (Windows API 前台窗口检测)
 *   - Data:       RandomAccessDatabase (二进制随机文件读写)
 */

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include "core/FocusSession.h"
#include "core/FocusController.h"
#include "data/RandomAccessDatabase.h"
#include "monitor/SystemMonitor.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FocusForest"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    // ─── 初始化各模块 ──────────────────────────────────────

    // 1. 数据文件路径（存放在用户数据目录）
    QString dataDir = QStandardPaths::writableLocation(
                          QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + QStringLiteral("/focus_records.dat");

    // 2. 创建核心组件
    FocusSession         session;
    RandomAccessDatabase database(dbPath);
    SystemMonitor        monitor;

    // 3. 打开/创建数据库文件
    if (!database.open()) {
        qCritical() << "无法打开数据库文件:" << dbPath;
        return 1;
    }

    // 4. 创建控制器（连接 Model、Data、Monitor）
    FocusController controller(&session, &database, &monitor);

    // 5. 创建主窗口（View）
    MainWindow window(&controller, &session, &monitor);
    window.show();

    // ─── 运行应用 ──────────────────────────────────────────
    int ret = app.exec();

    // ─── 清理 ──────────────────────────────────────────────
    database.close();
    return ret;
}
