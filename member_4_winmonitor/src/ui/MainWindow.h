#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QProgressBar>
#include <QTextEdit>
#include "core/FocusController.h"
#include "core/ForestLayoutManager.h"
#include "core/AchievementManager.h"
#include "core/ResinManager.h"
#include "core/GachaManager.h"

class GardenCanvas;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
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

    void onFocusModeChanged(int index);
    void onGentleViolation(int count);
    void onCoinsEarned(int amount, int balance);

    void onOpenStore();
    void onOpenMyForest();      // [新增] 我的森林
    void onOpenAchievements();  // [新增] 成就
    void onOpenGacha();         // [新增] 异色发掘


    void onAchievementUnlocked(int index, const QString& id);  // [新增]

private:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void setupUI();
    void setupConnections();
    void applyStyles();
    QString formatTime(int seconds) const;
    void logMessage(const QString& msg);
    void refreshPlantCombo();
    void refreshGarden();        // [新增] 刷新森林画布
    void checkAchievements(bool sessionJustSucceeded = false);    // [新增] 检测成就

    AchievementManager* m_achievements;   // [新增] 成就管理器
    ResinManager*       m_resins;          // [新增] 树脂
    GachaManager*       m_gacha;           // [新增] 抽卡

    QLabel*      m_timerLabel;
    QLabel*      m_stateLabel;
    QLabel*      m_plantLabel;
    QLabel*      m_procLabel;
    QProgressBar*m_growthBar;
    QPushButton* m_startBtn;
    QPushButton* m_pauseBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_blacklistBtn;
    QComboBox*   m_plantCombo;
    QSlider*     m_durationSlider;
    QLabel*      m_durationLabel;
    QTextEdit*   m_logOutput;
    GardenCanvas*m_gardenCanvas;

    QComboBox*   m_modeCombo;
    QLabel*      m_violationLabel;
    QLabel*      m_coinLabel;
    QPushButton* m_storeBtn;
    QPushButton* m_gachaBtn;        // [新增] 异色发掘

    QPushButton* m_forestBtn;       // [新增] 我的森林
    QPushButton* m_achievementBtn;  // [新增] 成就

    ForestLayoutManager* m_forestLayout;  // [新增] 森林布局持久化

    FocusController* m_controller;
    FocusSession*    m_session;
    SystemMonitor*   m_monitor;
};

#endif // MAINWINDOW_H
