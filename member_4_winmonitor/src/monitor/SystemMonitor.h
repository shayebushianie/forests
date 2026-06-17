#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QWaitCondition>
#include <atomic>

#include "RuleEngine.h"  // [新增] 引入规则引擎

/**
 * @brief 系统前台窗口监控器
 *
 * 职责（课程要求：Windows API 监听）：
 * 1. 独立线程运行，不阻塞 Qt 主 UI 线程
 * 2. 定时轮询 GetForegroundWindow() 获取当前前台窗口
 * 3. 与黑白名单进行匹配，发现违规窗口时发射 signal
 *
 * [修改] 黑白名单匹配逻辑委托给 RuleEngine 类
 * [修改] getForegroundProcessName() 增加管理员权限进程 fallback
 */
class SystemMonitor : public QThread {
    Q_OBJECT

public:
    explicit SystemMonitor(QObject* parent = nullptr);
    ~SystemMonitor() override;

    // ─── 黑白名单配置（委托 RuleEngine） ────────────────────
    // [修改] 以下方法内部委托给 m_ruleEngine，API 签名不变

    void setBlacklist(const QStringList& list);
    void setWhitelist(const QStringList& list);
    void setUseWhitelist(bool use);

    QStringList blacklist() const;
    QStringList whitelist() const;
    bool isUsingWhitelist() const { return m_ruleEngine.mode() == RuleEngine::Mode::Whitelist; }

    void addBlacklistItem(const QString& item);
    void addWhitelistItem(const QString& item);

    // ─── 控制接口 ──────────────────────────────────────────

    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const { return m_active; }

    /**
     * @brief 获取当前前台窗口进程名（Windows API 封装）
     * @return 进程可执行文件名（如 "chrome.exe"）
     *
     * [修改] 当 OpenProcess 因权限不足失败时，
     *         通过 GetWindowTextW() 获取窗口标题作为后备
     */
    static QString getForegroundProcessName();

signals:
    void windowViolation();
    void foregroundProcessChanged(const QString& processName);

protected:
    void run() override;

private:
    /// [修改] 委托给 m_ruleEngine.isViolation()
    bool isViolation(const QString& processName) const;

    mutable QMutex   m_mutex;
    QWaitCondition   m_condition;
    std::atomic<bool> m_active{false};
    std::atomic<bool> m_stopRequested{false};

    // [修改] 用 RuleEngine 对象替代原始的黑白名单列表
    RuleEngine  m_ruleEngine;
};

#endif // SYSTEMMONITOR_H
