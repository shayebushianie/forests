#include "MainWindow.h"
#include "GardenCanvas.h"
#include "monitor/SystemMonitor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(FocusController* controller,
                       FocusSession* session,
                       SystemMonitor* monitor,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_controller(controller)
    , m_session(session)
    , m_monitor(monitor) {

    setupUI();
    setupConnections();
    applyStyles();

    // 初始化显示
    onStateChanged(SessionState::Idle);
    onGrowthUpdated(0);
}

MainWindow::~MainWindow() = default;

// ════════════════════════════════════════════════════════════
// UI 搭建
// ════════════════════════════════════════════════════════════

void MainWindow::setupUI() {
    setWindowTitle(QStringLiteral("Focus Forest — 专注森林"));
    resize(800, 700);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ─── 1. 倒计时显示区 ────────────────────────────────────
    m_timerLabel = new QLabel(QStringLiteral("25:00"), this);
    m_timerLabel->setAlignment(Qt::AlignCenter);
    QFont timerFont = m_timerLabel->font();
    timerFont.setPointSize(56);
    timerFont.setBold(true);
    m_timerLabel->setFont(timerFont);

    m_stateLabel = new QLabel(QStringLiteral("🌱 待开始"), this);
    m_stateLabel->setAlignment(Qt::AlignCenter);
    QFont stateFont = m_stateLabel->font();
    stateFont.setPointSize(14);
    m_stateLabel->setFont(stateFont);

    mainLayout->addWidget(m_timerLabel);
    mainLayout->addWidget(m_stateLabel);

    // ─── 2. 植物状态与进度区 ────────────────────────────────
    QGroupBox* plantGroup = new QGroupBox(QStringLiteral("植物状态"), this);
    QHBoxLayout* plantLayout = new QHBoxLayout(plantGroup);

    m_plantLabel = new QLabel(QStringLiteral("🌳"), this);
    QFont plantFont = m_plantLabel->font();
    plantFont.setPointSize(36);
    m_plantLabel->setFont(plantFont);
    plantLayout->addWidget(m_plantLabel);

    m_growthBar = new QProgressBar(this);
    m_growthBar->setRange(0, 100);
    m_growthBar->setValue(0);
    m_growthBar->setTextVisible(true);
    m_growthBar->setFormat(QStringLiteral("生长 %1%"));
    plantLayout->addWidget(m_growthBar, 1);

    mainLayout->addWidget(plantGroup);

    // ─── 3. 控件区 ──────────────────────────────────────────
    QGroupBox* ctrlGroup = new QGroupBox(QStringLiteral("控制"), this);
    QVBoxLayout* ctrlLayout = new QVBoxLayout(ctrlGroup);

    // 3a. 植物选择 + 时长滑块
    QHBoxLayout* settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(new QLabel(QStringLiteral("植物:"), this));

    m_plantCombo = new QComboBox(this);
    m_plantCombo->addItem(QStringLiteral("🌳 橡树"),     QStringLiteral("OakTree"));
    m_plantCombo->addItem(QStringLiteral("🌲 松树"),     QStringLiteral("PineTree"));
    m_plantCombo->addItem(QStringLiteral("🌹 玫瑰"),     QStringLiteral("Rose"));
    m_plantCombo->addItem(QStringLiteral("🌻 向日葵"),   QStringLiteral("Sunflower"));
    settingsLayout->addWidget(m_plantCombo);

    settingsLayout->addWidget(new QLabel(QStringLiteral("时长:"), this));

    m_durationSlider = new QSlider(Qt::Horizontal, this);
    m_durationSlider->setRange(1, 120);   // 1~120 分钟
    m_durationSlider->setValue(25);
    m_durationSlider->setTickPosition(QSlider::TicksBelow);
    m_durationSlider->setTickInterval(5);
    settingsLayout->addWidget(m_durationSlider, 1);

    m_durationLabel = new QLabel(QStringLiteral("25:00"), this);
    settingsLayout->addWidget(m_durationLabel);

    ctrlLayout->addLayout(settingsLayout);

    // 3b. 按钮行
    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_startBtn = new QPushButton(QStringLiteral("▶ 开始"), this);
    m_pauseBtn = new QPushButton(QStringLiteral("⏸ 暂停"), this);
    m_resetBtn = new QPushButton(QStringLiteral("⏹ 重置"), this);

    m_pauseBtn->setEnabled(false);

    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_pauseBtn);
    btnLayout->addWidget(m_resetBtn);

    // 黑名单按钮 + 前台进程显示
    m_blacklistBtn = new QPushButton(QStringLiteral("加入黑名单"), this);
    m_procLabel = new QLabel(QStringLiteral("当前进程: —"), this);
    btnLayout->addWidget(m_blacklistBtn);
    btnLayout->addWidget(m_procLabel, 1);

    ctrlLayout->addLayout(btnLayout);

    mainLayout->addWidget(ctrlGroup);

    // ─── 4. 森林画布 ────────────────────────────────────────
    m_gardenCanvas = new GardenCanvas(this);
    mainLayout->addWidget(m_gardenCanvas, 1);

    // ─── 5. 日志输出 ────────────────────────────────────────
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(120);
    m_logOutput->setPlaceholderText(QStringLiteral("日志信息..."));
    mainLayout->addWidget(m_logOutput);
}

// ════════════════════════════════════════════════════════════
// 信号连接
// ════════════════════════════════════════════════════════════

void MainWindow::setupConnections() {
    // 按钮点击 → Controller
    connect(m_startBtn, &QPushButton::clicked,
            this, &MainWindow::onStartClicked);
    connect(m_pauseBtn, &QPushButton::clicked,
            this, &MainWindow::onPauseClicked);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &MainWindow::onResetClicked);
    connect(m_plantCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPlantTypeChanged);
    connect(m_durationSlider, &QSlider::valueChanged,
            this, &MainWindow::onDurationSliderChanged);
    connect(m_blacklistBtn, &QPushButton::clicked,
            this, &MainWindow::onAddToBlacklist);

    // Controller 信号 → View 刷新
    connect(m_controller, &FocusController::timeUpdated,
            this, &MainWindow::onTimeUpdated);
    connect(m_controller, &FocusController::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(m_controller, &FocusController::growthUpdated,
            this, &MainWindow::onGrowthUpdated);

    // SystemMonitor 信号 → View
    connect(m_monitor, &SystemMonitor::foregroundProcessChanged,
            this, &MainWindow::onForegroundProcessChanged);
}

// ════════════════════════════════════════════════════════════
// QSS 样式
// ════════════════════════════════════════════════════════════

void MainWindow::applyStyles() {
    setStyleSheet(QStringLiteral(R"(
        QMainWindow {
            background-color: #f0f8f0;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #a0c0a0;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 16px;
            background-color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #2d6a2d;
        }
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 18px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #888888;
        }
        QPushButton#blacklistBtn {
            background-color: #d32f2f;
        }
        QPushButton#blacklistBtn:hover {
            background-color: #b71c1c;
        }
        QProgressBar {
            border: 1px solid #a0c0a0;
            border-radius: 6px;
            text-align: center;
            height: 22px;
            background-color: #e0f0e0;
        }
        QProgressBar::chunk {
            background-color: #4caf50;
            border-radius: 5px;
        }
        QComboBox, QSlider {
            font-size: 13px;
        }
        QLabel {
            color: #2d2d2d;
        }
        QTextEdit {
            border: 1px solid #a0c0a0;
            border-radius: 6px;
            background-color: #fafafa;
            font-family: Consolas, monospace;
            font-size: 11px;
        }
    )"));

    m_blacklistBtn->setObjectName(QStringLiteral("blacklistBtn"));
}

// ════════════════════════════════════════════════════════════
// Slots
// ════════════════════════════════════════════════════════════

void MainWindow::onStartClicked() {
    m_controller->startFocus();
    logMessage(QStringLiteral("专注开始！种植: ") + m_plantCombo->currentText());
}

void MainWindow::onPauseClicked() {
    if (m_pauseBtn->text().contains(QStringLiteral("暂停"))) {
        m_controller->pauseFocus();
        m_pauseBtn->setText(QStringLiteral("▶ 继续"));
        logMessage(QStringLiteral("专注已暂停"));
    } else {
        m_controller->resumeFocus();
        m_pauseBtn->setText(QStringLiteral("⏸ 暂停"));
        logMessage(QStringLiteral("专注已恢复"));
    }
}

void MainWindow::onResetClicked() {
    m_controller->resetFocus();
    m_pauseBtn->setText(QStringLiteral("⏸ 暂停"));
    logMessage(QStringLiteral("已重置"));
}

void MainWindow::onPlantTypeChanged(int index) {
    QString type = m_plantCombo->itemData(index).toString();
    m_controller->setPlantType(type);
}

void MainWindow::onDurationSliderChanged(int value) {
    int seconds = value * 60;
    m_session->setTotalSeconds(seconds);
    m_durationLabel->setText(formatTime(seconds));
    m_timerLabel->setText(formatTime(seconds));
}

void MainWindow::onTimeUpdated(int remaining) {
    m_timerLabel->setText(formatTime(remaining));
}

void MainWindow::onStateChanged(SessionState state) {
    switch (state) {
    case SessionState::Idle:
        m_stateLabel->setText(QStringLiteral("🌱 待开始"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_pauseBtn->setText(QStringLiteral("⏸ 暂停"));
        m_plantCombo->setEnabled(true);
        m_durationSlider->setEnabled(true);
        break;
    case SessionState::Focusing:
        m_stateLabel->setText(QStringLiteral("🌿 专注中..."));
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        m_plantCombo->setEnabled(false);
        m_durationSlider->setEnabled(false);
        break;
    case SessionState::Paused:
        m_stateLabel->setText(QStringLiteral("⏸ 已暂停"));
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        break;
    case SessionState::Success:
        m_stateLabel->setText(QStringLiteral("🎉 专注成功！"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_plantCombo->setEnabled(true);
        m_durationSlider->setEnabled(true);
        break;
    case SessionState::Failed:
        m_stateLabel->setText(QStringLiteral("💀 专注失败 (违规)"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_plantCombo->setEnabled(true);
        m_durationSlider->setEnabled(true);
        break;
    }
}

void MainWindow::onGrowthUpdated(int percent) {
    m_growthBar->setValue(percent);

    // 更新植物图标
    auto* plant = m_controller->currentPlant();
    if (plant) {
        QString emoji;
        switch (plant->stage()) {
        case GrowthStage::Seed:     emoji = QStringLiteral("🌰"); break;
        case GrowthStage::Sprout:   emoji = QStringLiteral("🌱"); break;
        case GrowthStage::Growing:  emoji = QStringLiteral("🌿"); break;
        case GrowthStage::Mature:   emoji = QStringLiteral("🌳"); break;
        case GrowthStage::Withered: emoji = QStringLiteral("🍂"); break;
        }
        m_plantLabel->setText(emoji);
    }
}

void MainWindow::onForegroundProcessChanged(const QString& processName) {
    m_procLabel->setText(QStringLiteral("当前进程: ") + processName);
}

void MainWindow::onAddToBlacklist() {
    QString current = SystemMonitor::getForegroundProcessName();
    if (!current.isEmpty()) {
        m_monitor->addBlacklistItem(current);
        logMessage(QStringLiteral("已加入黑名单: ") + current);
    }
}

// ════════════════════════════════════════════════════════════
// 工具方法
// ════════════════════════════════════════════════════════════

QString MainWindow::formatTime(int seconds) const {
    int m = seconds / 60;
    int s = seconds % 60;
    return QStringLiteral("%1:%2")
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'));
}

void MainWindow::logMessage(const QString& msg) {
    QString timestamp = QDateTime::currentDateTime()
                            .toString(QStringLiteral("HH:mm:ss"));
    m_logOutput->append(QStringLiteral("[%1] %2").arg(timestamp, msg));
}
