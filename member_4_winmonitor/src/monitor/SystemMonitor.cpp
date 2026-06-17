#include "SystemMonitor.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <QDebug>

SystemMonitor::SystemMonitor(QObject* parent)
    : QThread(parent) {
}

SystemMonitor::~SystemMonitor() {
    stopMonitoring();
    wait(3000);
}

// ─── 以下方法委托给 m_ruleEngine ─────────────────────────────
// [修改] 原直接操作 m_blacklist/m_whitelist，现委托 RuleEngine

void SystemMonitor::setBlacklist(const QStringList& list) {
    m_ruleEngine.setBlacklist(list);
}

void SystemMonitor::setWhitelist(const QStringList& list) {
    m_ruleEngine.setWhitelist(list);
}

void SystemMonitor::setUseWhitelist(bool use) {
    m_ruleEngine.setMode(use ? RuleEngine::Mode::Whitelist
                             : RuleEngine::Mode::Blacklist);
}

QStringList SystemMonitor::blacklist() const {
    return m_ruleEngine.blacklist();
}

QStringList SystemMonitor::whitelist() const {
    return m_ruleEngine.whitelist();
}

void SystemMonitor::addBlacklistItem(const QString& item) {
    m_ruleEngine.addBlacklistItem(item);
}

void SystemMonitor::addWhitelistItem(const QString& item) {
    m_ruleEngine.addWhitelistItem(item);
}

// ─── 控制接口 ────────────────────────────────────────────────

void SystemMonitor::startMonitoring() {
    m_stopRequested = false;
    m_active = true;

    if (!isRunning()) {
        start();
    } else {
        m_condition.wakeAll();
    }

    qDebug() << "[SystemMonitor] 监控已启动";
}

void SystemMonitor::stopMonitoring() {
    m_active = false;
    m_stopRequested = true;
    m_condition.wakeAll();
    qDebug() << "[SystemMonitor] 监控已停止";
}

// ─── Windows API 前台窗口检测 ────────────────────────────────

QString SystemMonitor::getForegroundProcessName() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return {};

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) return {};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) {
        // [新增] 管理员权限进程 fallback: 通过窗口标题识别
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            wchar_t title[256];
            int len = GetWindowTextW(hwnd, title, 256);
            if (len > 0) {
                QString result = QString::fromWCharArray(title, len);
                qDebug() << "[SystemMonitor] 管理员进程(窗口标题):" << result;
                return QStringLiteral("SYSTEM_ELEVATED");
            }
        }
        return {};
    }

    wchar_t path[MAX_PATH];
    DWORD size = MAX_PATH;
    QString result;

    if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
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
    // [修改] 委托给 RuleEngine
    return m_ruleEngine.isViolation(processName);
}

void SystemMonitor::run() {
    qDebug() << "[SystemMonitor] 监控线程已启动";

    QString lastProcess;

    while (!m_stopRequested) {
        if (!m_active) {
            QMutexLocker locker(&m_mutex);
            m_condition.wait(&m_mutex, 1000);
            continue;
        }

        QString procName = getForegroundProcessName();

        if (procName != lastProcess) {
            lastProcess = procName;
            emit foregroundProcessChanged(procName);
        }

        if (isViolation(procName)) {
            qDebug() << "[SystemMonitor] 违规窗口:" << procName;
            emit windowViolation();
        }

        QThread::sleep(1);
    }

    qDebug() << "[SystemMonitor] 监控线程已退出";
}
