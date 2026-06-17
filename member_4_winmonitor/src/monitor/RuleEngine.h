#ifndef RULEENGINE_H
#define RULEENGINE_H

#include <QString>
#include <QStringList>
#include <QMutex>

/**
 * @brief 黑白名单规则引擎
 *
 * [新增] 从 SystemMonitor 中独立出的规则匹配模块。
 * 职责：
 * 1. 管理黑名单 / 白名单列表（线程安全）
 * 2. 判断给定进程名是否违规
 * 3. 支持黑名单模式（默认）和白名单模式
 */
class RuleEngine {
public:
    explicit RuleEngine();
    ~RuleEngine() = default;

    // ─── 模式设置 ──────────────────────────────────────────
    enum class Mode { Blacklist, Whitelist };

    void setMode(Mode mode);
    Mode mode() const { return m_mode; }

    // ─── 黑白名单配置 ──────────────────────────────────────
    void setBlacklist(const QStringList& list);
    void setWhitelist(const QStringList& list);
    QStringList blacklist() const;
    QStringList whitelist() const;

    void addBlacklistItem(const QString& item);
    void addWhitelistItem(const QString& item);

    // ─── 规则判定 ──────────────────────────────────────────
    bool isViolation(const QString& processName) const;

private:
    mutable QMutex m_mutex;
    Mode           m_mode;
    QStringList    m_blacklist;
    QStringList    m_whitelist;
};

#endif // RULEENGINE_H
