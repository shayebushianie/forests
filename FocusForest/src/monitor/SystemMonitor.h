#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QWaitCondition>
#include <atomic>

/**
 * @brief 系统前台窗口监控器
 *
 * 职责（课程要求：Windows API 监听）：
 * 1. 独立线程运行，不阻塞 Qt 主 UI 线程
 * 2. 定时轮询 GetForegroundWindow() 获取当前前台窗口
 * 3. 与黑白名单进行匹配，发现违规窗口时发射 signal
 *
 * 技术实现：
 * - 轮询周期: 1000ms（平衡性能与响应速度）
 * - 通过 QThread 的 run() 事件循环驱动 QTimer
 *
 * 黑白名单匹配规则：
 * - 黑名单：进程名包含在黑名单列表中 → 违规
 * - 白名单（可选）：仅当进程名在白名单中时才允许
 */
class SystemMonitor : public QThread {
    Q_OBJECT

public:
    explicit SystemMonitor(QObject* parent = nullptr);
    ~SystemMonitor() override;

    // ─── 黑白名单配置 ──────────────────────────────────────

    void setBlacklist(const QStringList& list);
    void setWhitelist(const QStringList& list);
    void setUseWhitelist(bool use) { m_useWhitelist = use; }

    QStringList blacklist() const { return m_blacklist; }
    QStringList whitelist() const { return m_whitelist; }
    bool isUsingWhitelist() const { return m_useWhitelist; }

    /// 添加单个黑名单条目
    void addBlacklistItem(const QString& item);
    /// 添加单个白名单条目
    void addWhitelistItem(const QString& item);

    // ─── 控制接口 ──────────────────────────────────────────

    /// 开始监控（启动线程，注意只能启动一次）
    void startMonitoring();
    /// 停止监控
    void stopMonitoring();
    /// 是否正在监控
    bool isMonitoring() const { return m_active; }

    /**
     * @brief 获取当前前台窗口进程名（Windows API 封装）
     * @return 进程可执行文件名（如 "chrome.exe"）
     *
     * 步骤：
     * 1. GetForegroundWindow() 获取前台窗口句柄
     * 2. GetWindowThreadProcessId() 获取 PID
     * 3. OpenProcess() + QueryFullProcessImageNameW() 获取路径
     * 4. 提取文件名部分
     */
    static QString getForegroundProcessName();

signals:
    /// 检测到违规窗口时发射
    void windowViolation();
    /// 前台窗口变更通知（用于 UI 显示当前进程名）
    void foregroundProcessChanged(const QString& processName);

protected:
    void run() override;

private:
    /// 检查指定进程名是否违规
    bool isViolation(const QString& processName) const;

    mutable QMutex   m_mutex;
    QWaitCondition   m_condition;
    std::atomic<bool> m_active{false};
    std::atomic<bool> m_stopRequested{false};

    QStringList m_blacklist;    ///< 黑名单进程名列表
    QStringList m_whitelist;    ///< 白名单进程名列表
    bool        m_useWhitelist; ///< 是否启用白名单模式
};

#endif // SYSTEMMONITOR_H
