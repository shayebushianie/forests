#include "SystemMonitor.h"

#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#include <QDebug>

SystemMonitor::SystemMonitor(QObject* parent)
    : QThread(parent)
    , m_useWhitelist(false) {

    // 默认黑名单（示例）
    m_blacklist << QStringLiteral("chrome.exe")
                << QStringLiteral("firefox.exe")
                << QStringLiteral("msedge.exe")
                << QStringLiteral("LeagueOfLegends.exe")
                << QStringLiteral("QQ.exe")
                << QStringLiteral("WeChat.exe");

    // 默认白名单（开发工具）
    m_whitelist << QStringLiteral("FocusForest.exe")
                << QStringLiteral("devenv.exe")
                << QStringLiteral("code.exe")
                << QStringLiteral("clion64.exe");
}

SystemMonitor::~SystemMonitor() {
    stopMonitoring();
    wait(3000);  // 等待线程结束
}

void SystemMonitor::setBlacklist(const QStringList& list) {
    QMutexLocker locker(&m_mutex);
    m_blacklist = list;
}

void SystemMonitor::setWhitelist(const QStringList& list) {
    QMutexLocker locker(&m_mutex);
    m_whitelist = list;
}

void SystemMonitor::addBlacklistItem(const QString& item) {
    QMutexLocker locker(&m_mutex);
    if (!m_blacklist.contains(item, Qt::CaseInsensitive)) {
        m_blacklist.append(item);
    }
}

void SystemMonitor::addWhitelistItem(const QString& item) {
    QMutexLocker locker(&m_mutex);
    if (!m_whitelist.contains(item, Qt::CaseInsensitive)) {
        m_whitelist.append(item);
    }
}

void SystemMonitor::startMonitoring() {
    m_stopRequested = false;
    m_active = true;

    if (!isRunning()) {
        start();  // 启动线程
    } else {
        m_condition.wakeAll();  // 唤醒等待的线程
    }

    qDebug() << "[SystemMonitor] 监控已启动";
}

void SystemMonitor::stopMonitoring() {
    m_active = false;
    m_stopRequested = true;
    m_condition.wakeAll();
    qDebug() << "[SystemMonitor] 监控已停止";
}

QString SystemMonitor::getForegroundProcessName() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return {};

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) return {};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return {};

    wchar_t path[MAX_PATH];
    DWORD size = MAX_PATH;
    QString result;

    if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
        // 提取文件名（去掉路径）
        std::wstring ws(path);
        auto pos = ws.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            ws = ws.substr(pos + 1);
        }
        result = QString::fromStdWString(ws);
    }

    CloseHandle(hProcess);
    return result;
}

bool SystemMonitor::isViolation(const QString& processName) const {
    if (processName.isEmpty()) return false;

    if (m_useWhitelist) {
        // 白名单模式：仅允许白名单内的进程
        for (const auto& allowed : m_whitelist) {
            if (processName.compare(allowed, Qt::CaseInsensitive) == 0) {
                return false;  // 在白名单中 → 不违规
            }
        }
        return true;  // 不在白名单中 → 违规
    }

    // 黑名单模式：检测是否在黑名单中
    for (const auto& banned : m_blacklist) {
        if (processName.contains(banned, Qt::CaseInsensitive)) {
            return true;  // 在黑名单中 → 违规
        }
    }
    return false;
}

void SystemMonitor::run() {
    qDebug() << "[SystemMonitor] 监控线程已启动";

    QString lastProcess;

    while (!m_stopRequested) {
        if (!m_active) {
            // 如果未激活，等待唤醒
            QMutexLocker locker(&m_mutex);
            m_condition.wait(&m_mutex, 1000);
            continue;
        }

        // 获取当前前台进程名
        QString procName = getForegroundProcessName();

        // 如果进程名变化，发射信号
        if (procName != lastProcess) {
            lastProcess = procName;
            emit foregroundProcessChanged(procName);
        }

        // 检测违规
        if (isViolation(procName)) {
            qDebug() << "[SystemMonitor] 违规窗口:" << procName;
            emit windowViolation();
        }

        // 休眠 1 秒（轮询间隔）
        QThread::sleep(1);
    }

    qDebug() << "[SystemMonitor] 监控线程已退出";
}
