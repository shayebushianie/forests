#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QProgressBar>
#include <QTextEdit>
#include "FocusController.h"

class GardenCanvas;

/**
 * @brief 主窗口 —— MVC 中的 View
 *
 * 职责：
 * 1. 显示倒计时、植物状态、历史森林
 * 2. 接收用户操作（开始/暂停/重置/设置）
 * 3. 转发用户事件到 Controller
 * 4. 显示 SystemMonitor 的前台进程信息
 *
 * UI 布局（垂直排列）：
 * ┌─────────────────────────────────┐
 * │         倒计时显示区            │
 * ├─────────────────────────────────┤
 * │     植物状态/进度条区           │
 * ├─────────────────────────────────┤
 * │  控件区 (开始/暂停/重置/设置)    │
 * ├─────────────────────────────────┤
 * │         森林画布                │
 * ├─────────────────────────────────┤
 * │      日志/进程信息区            │
 * └─────────────────────────────────┘
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param controller 专注控制器
     * @param session    专注会话
     * @param monitor    系统监控器
     * @param parent     父窗口
     */
    explicit MainWindow(FocusController* controller,
                        FocusSession* session,
                        SystemMonitor* monitor,
                        QWidget* parent = nullptr);

    ~MainWindow() override;

private slots:
    void onStartClicked();
    void onPauseClicked();
    void onResetClicked();
    void onPlantTypeChanged(int index);
    void onDurationSliderChanged(int value);

    void onTimeUpdated(int remainingSeconds);
    void onStateChanged(SessionState state);
    void onGrowthUpdated(int percent);
    void onForegroundProcessChanged(const QString& processName);
    void onAddToBlacklist();

private:
    void setupUI();
    void setupConnections();
    void applyStyles();
    QString formatTime(int seconds) const;
    void logMessage(const QString& msg);

    // ─── UI 组件 ───────────────────────────────────────────
    QLabel*      m_timerLabel;       // 倒计时显示
    QLabel*      m_stateLabel;       // 状态文字
    QLabel*      m_plantLabel;       // 当前植物图标
    QLabel*      m_procLabel;        // 当前前台进程名
    QProgressBar*m_growthBar;        // 植物生长进度条
    QPushButton* m_startBtn;
    QPushButton* m_pauseBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_blacklistBtn;
    QComboBox*   m_plantCombo;       // 植物类型选择
    QSlider*     m_durationSlider;   // 专注时长滑块
    QLabel*      m_durationLabel;    // "25:00"
    QTextEdit*   m_logOutput;        // 日志输出
    GardenCanvas*m_gardenCanvas;     // 历史森林画布

    // ─── 核心对象（非 owning） ──────────────────────────────
    FocusController* m_controller;
    FocusSession*    m_session;
    SystemMonitor*   m_monitor;
};

#endif // MAINWINDOW_H
