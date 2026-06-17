#include "RuleEngine.h"
#include <QDebug>

RuleEngine::RuleEngine()
    : m_mode(Mode::Blacklist) {

    // 默认黑名单
    m_blacklist << QStringLiteral("chrome.exe")
                << QStringLiteral("firefox.exe")
                << QStringLiteral("msedge.exe")
                << QStringLiteral("LeagueOfLegends.exe")
                << QStringLiteral("QQ.exe")
                << QStringLiteral("WeChat.exe");

    // 默认白名单
    m_whitelist << QStringLiteral("FocusForest.exe")
                << QStringLiteral("devenv.exe")
                << QStringLiteral("code.exe")
                << QStringLiteral("clion64.exe");
}

void RuleEngine::setMode(Mode mode) {
    QMutexLocker locker(&m_mutex);
    m_mode = mode;
}

void RuleEngine::setBlacklist(const QStringList& list) {
    QMutexLocker locker(&m_mutex);
    m_blacklist = list;
}

void RuleEngine::setWhitelist(const QStringList& list) {
    QMutexLocker locker(&m_mutex);
    m_whitelist = list;
}

QStringList RuleEngine::blacklist() const {
    QMutexLocker locker(&m_mutex);
    return m_blacklist;
}

QStringList RuleEngine::whitelist() const {
    QMutexLocker locker(&m_mutex);
    return m_whitelist;
}

void RuleEngine::addBlacklistItem(const QString& item) {
    QMutexLocker locker(&m_mutex);
    if (!m_blacklist.contains(item, Qt::CaseInsensitive)) {
        m_blacklist.append(item);
    }
}

void RuleEngine::addWhitelistItem(const QString& item) {
    QMutexLocker locker(&m_mutex);
    if (!m_whitelist.contains(item, Qt::CaseInsensitive)) {
        m_whitelist.append(item);
    }
}

bool RuleEngine::isViolation(const QString& processName) const {
    if (processName.isEmpty()) return false;

    QMutexLocker locker(&m_mutex);

    if (m_mode == Mode::Whitelist) {
        // 白名单模式：仅允许白名单内的进程
        for (const auto& allowed : m_whitelist) {
            if (processName.compare(allowed, Qt::CaseInsensitive) == 0) {
                return false;
            }
        }
        return true;
    }

    // 黑名单模式（默认）
    for (const auto& banned : m_blacklist) {
        if (processName.contains(banned, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
