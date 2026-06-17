/**
 * @file main.cpp
 * @brief Focus Forest 专注森林 —— 程序入口
 */

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <cstring>
#include <ctime>

#include "core/FocusSession.h"
#include "core/FocusController.h"
#include "data/RandomAccessDatabase.h"
#include "monitor/SystemMonitor.h"
#include "ui/MainWindow.h"

static void checkCrashRecovery(RandomAccessDatabase& database) {
    if (database.recordCount() == 0) return;

    FocusRecord lastRecord;
    if (!database.readRecord(database.recordCount() - 1, lastRecord)) return;

    if (lastRecord.isSuccess == 1) return;
    if (lastRecord.actualSeconds >= lastRecord.durationSeconds) return;

    FocusRecord crashRecord;
    crashRecord.recordId = database.recordCount();
    std::strncpy(crashRecord.plantType, "CrashRecovery",
                 sizeof(crashRecord.plantType) - 1);
    crashRecord.durationSeconds = lastRecord.durationSeconds;
    crashRecord.actualSeconds   = lastRecord.actualSeconds;
    crashRecord.timestamp       = std::time(nullptr);
    crashRecord.isSuccess       = 0;

    if (database.writeRecord(crashRecord)) {
        qInfo() << "[CrashRecovery] 检测到上次异常退出，已追加枯萎恢复记录";
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FocusForest"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    QString dataDir = QStandardPaths::writableLocation(
                          QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath        = dataDir + QStringLiteral("/focus_records.dat");
    QString coinPath      = dataDir + QStringLiteral("/coins.dat");
    QString storePath     = dataDir + QStringLiteral("/store.dat");   // [新增]
    QString inProgressPath= dataDir + QStringLiteral("/inprogress.dat"); // [新增]

    FocusSession         session;
    RandomAccessDatabase database(dbPath);
    SystemMonitor        monitor;

    if (!database.open()) {
        qCritical() << "无法打开数据库文件:" << dbPath;
        return 1;
    }

    checkCrashRecovery(database);

    // [修改] 传递 store + inProgress 文件路径
    FocusController controller(&session, &database, &monitor,
                               coinPath, storePath, inProgressPath);

    MainWindow window(&controller, &session, &monitor);
    window.show();

    int ret = app.exec();

    database.close();
    return ret;
}
