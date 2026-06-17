#include "MainWindow.h"
#include "GardenCanvas.h"
#include "StoreDialog.h"

#include "MyForestDialog.h"
#include "AchievementDialog.h"
#include "AchievementToast.h"
#include "monitor/SystemMonitor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QEvent>
#include <QStackedWidget>
#include <functional>

MainWindow::MainWindow(FocusController* controller,
                       FocusSession* session,
                       SystemMonitor* monitor,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_controller(controller)
    , m_session(session)
    , m_monitor(monitor) {

    // [新增] 森林布局持久化
    QString dataDir = QStandardPaths::writableLocation(
                          QStandardPaths::AppDataLocation);
    m_forestLayout = new ForestLayoutManager(
        dataDir + QStringLiteral("/forest_layout.dat"));
    m_achievements = new AchievementManager(
        dataDir + QStringLiteral("/achievements.dat"), this);
    m_resins = new ResinManager(
        dataDir + QStringLiteral("/resins.dat"));
    m_gacha = new GachaManager(
        dataDir + QStringLiteral("/variants.dat"));

    setupUI();
    setupConnections();
    applyStyles();

    onStateChanged(SessionState::Idle);
    onGrowthUpdated(0);
    refreshPlantCombo();
    refreshGarden();

}

MainWindow::~MainWindow() {
    delete m_forestLayout;
    delete m_resins;
    delete m_gacha;
}

void MainWindow::setupUI() {
    setWindowTitle(QStringLiteral("Focus Forest — 专注森林"));
    resize(800, 760);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ─── 1. 倒计时 + 金币 ──────────────────────────────────
    m_timerLabel = new QLabel(QStringLiteral("25:00"), this);
    m_timerLabel->setAlignment(Qt::AlignCenter);
    QFont timerFont = m_timerLabel->font();
    timerFont.setPointSize(50);
    timerFont.setBold(true);
    m_timerLabel->setFont(timerFont);

    m_coinLabel = new QLabel(QStringLiteral("🪙 0"), this);
    m_coinLabel->setAlignment(Qt::AlignRight);
    QFont coinFont = m_coinLabel->font();
    coinFont.setPointSize(13);
    coinFont.setBold(true);
    m_coinLabel->setFont(coinFont);
    m_coinLabel->setStyleSheet(QStringLiteral("color: #f39c12; padding-right: 8px;"));

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addStretch();
    topRow->addWidget(m_coinLabel);
    mainLayout->addLayout(topRow);

    // [新增] 顶部按钮行：收集 + 商店
    QHBoxLayout* topBtns = new QHBoxLayout();
    topBtns->addStretch();

    m_forestBtn = new QPushButton(QStringLiteral("🌲 我的森林"), this);
    m_forestBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1b5e20; color: white;"
        " border: none; border-radius: 6px; padding: 6px 14px;"
        " font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #0d3b10; }"));
    topBtns->addWidget(m_forestBtn);

    m_achievementBtn = new QPushButton(QStringLiteral("🏆 成就"), this);
    m_achievementBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #b8860b; color: white;"
        " border: none; border-radius: 6px; padding: 6px 14px;"
        " font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #8b6508; }"));
    topBtns->addWidget(m_achievementBtn);

    m_storeBtn = new QPushButton(QStringLiteral("🏪 种子商店"), this);
    m_storeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f39c12; color: white;"
        " border: none; border-radius: 6px; padding: 6px 14px;"
        " font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #e67e22; }"));
    topBtns->addWidget(m_storeBtn);

    m_gachaBtn = new QPushButton(QStringLiteral("✨ 异色发掘"), this);
    m_gachaBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "  stop:0 #e74c3c, stop:1 #8e44ad); color: white;"
        " border: none; border-radius: 6px; padding: 6px 14px;"
        " font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "  stop:0 #c0392b, stop:1 #7d3c98); }"));
    topBtns->addWidget(m_gachaBtn);

    mainLayout->addLayout(topBtns);
    mainLayout->addWidget(m_timerLabel);

    m_stateLabel = new QLabel(QStringLiteral("🌱 待开始"), this);
    m_stateLabel->setAlignment(Qt::AlignCenter);
    QFont stateFont = m_stateLabel->font();
    stateFont.setPointSize(13);
    m_stateLabel->setFont(stateFont);
    mainLayout->addWidget(m_stateLabel);

    // ─── 2. 植物状态与进度区 ────────────────────────────────
    QGroupBox* plantGroup = new QGroupBox(QStringLiteral("🌱 植物状态"), this);
    QHBoxLayout* plantLayout = new QHBoxLayout(plantGroup);

    m_plantLabel = new QLabel(QStringLiteral("🌳"), this);
    QFont plantFont = m_plantLabel->font();
    plantFont.setPointSize(32);
    m_plantLabel->setFont(plantFont);
    plantLayout->addWidget(m_plantLabel);

    m_growthBar = new QProgressBar(this);
    m_growthBar->setRange(0, 100);
    m_growthBar->setValue(0);
    m_growthBar->setTextVisible(true);
    m_growthBar->setFormat(QStringLiteral("生长 %p%"));
    plantLayout->addWidget(m_growthBar, 1);

    mainLayout->addWidget(plantGroup);

    // ─── 3. 控件区 ──────────────────────────────────────────
    QGroupBox* ctrlGroup = new QGroupBox(QStringLiteral("🎛 控制"), this);
    QVBoxLayout* ctrlLayout = new QVBoxLayout(ctrlGroup);

    QHBoxLayout* settingsLayout = new QHBoxLayout();
    settingsLayout->setSpacing(6);

    settingsLayout->addWidget(new QLabel(QStringLiteral("植物:"), this));
    m_plantCombo = new QComboBox(this);
    settingsLayout->addWidget(m_plantCombo);

    settingsLayout->addWidget(new QLabel(QStringLiteral("模式:"), this));
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(QStringLiteral("🔴 严格"), QVariant::fromValue(0));
    m_modeCombo->addItem(QStringLiteral("🟡 温和"), QVariant::fromValue(1));
    m_modeCombo->setCurrentIndex(0);
    settingsLayout->addWidget(m_modeCombo);

    settingsLayout->addWidget(new QLabel(QStringLiteral("时长:"), this));
    m_durationSlider = new QSlider(Qt::Horizontal, this);
    m_durationSlider->setRange(1, 120);
    m_durationSlider->setValue(25);
    m_durationSlider->setTickPosition(QSlider::TicksBelow);
    m_durationSlider->setTickInterval(5);
    settingsLayout->addWidget(m_durationSlider, 1);

    m_durationLabel = new QLabel(QStringLiteral("25:00"), this);
    settingsLayout->addWidget(m_durationLabel);

    ctrlLayout->addLayout(settingsLayout);

    // 按钮行
    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_startBtn = new QPushButton(QStringLiteral("▶ 开始"), this);
    m_pauseBtn = new QPushButton(QStringLiteral("⏸ 暂停"), this);
    m_resetBtn = new QPushButton(QStringLiteral("⏹ 重置"), this);
    m_pauseBtn->setEnabled(false);

    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_pauseBtn);
    btnLayout->addWidget(m_resetBtn);

    m_blacklistBtn = new QPushButton(QStringLiteral("加入黑名单"), this);
    m_procLabel = new QLabel(QStringLiteral("进程: —"), this);
    btnLayout->addWidget(m_blacklistBtn);
    btnLayout->addWidget(m_procLabel, 1);

    ctrlLayout->addLayout(btnLayout);

    m_violationLabel = new QLabel(QStringLiteral(""), this);
    m_violationLabel->setAlignment(Qt::AlignCenter);
    m_violationLabel->setStyleSheet(
        QStringLiteral("color: #e67e22; font-weight: bold; font-size: 12px;"));
    m_violationLabel->hide();
    ctrlLayout->addWidget(m_violationLabel);

    mainLayout->addWidget(ctrlGroup);

    // ─── 4. 森林画布 ────────────────────────────────────────
    m_gardenCanvas = new GardenCanvas(this);
    m_gardenCanvas->setMinimumHeight(240);
    mainLayout->addWidget(m_gardenCanvas, 1);

    // ─── 5. 日志输出 ────────────────────────────────────────
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(80);
    m_logOutput->setPlaceholderText(QStringLiteral("日志..."));
    mainLayout->addWidget(m_logOutput);
}

void MainWindow::setupConnections() {
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(m_plantCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPlantTypeChanged);
    connect(m_durationSlider, &QSlider::valueChanged,
            this, &MainWindow::onDurationSliderChanged);
    connect(m_blacklistBtn, &QPushButton::clicked, this, &MainWindow::onAddToBlacklist);

    connect(m_forestBtn, &QPushButton::clicked, this, &MainWindow::onOpenMyForest);
    connect(m_achievementBtn, &QPushButton::clicked, this, &MainWindow::onOpenAchievements);
    connect(m_storeBtn, &QPushButton::clicked, this, &MainWindow::onOpenStore);
    connect(m_gachaBtn, &QPushButton::clicked, this, &MainWindow::onOpenGacha);

    connect(m_achievements, &AchievementManager::achievementUnlocked,
            this, &MainWindow::onAchievementUnlocked);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFocusModeChanged);

    connect(m_controller, &FocusController::timeUpdated, this, &MainWindow::onTimeUpdated);
    connect(m_controller, &FocusController::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_controller, &FocusController::growthUpdated, this, &MainWindow::onGrowthUpdated);

    connect(m_controller, &FocusController::gentleViolation, this, &MainWindow::onGentleViolation);
    connect(m_controller, &FocusController::coinsEarned, this, &MainWindow::onCoinsEarned);

    connect(m_monitor, &SystemMonitor::foregroundProcessChanged,
            this, &MainWindow::onForegroundProcessChanged);
}

void MainWindow::applyStyles() {
    setStyleSheet(QStringLiteral(R"(
        QMainWindow { background-color: #f0f8f0; }
        QGroupBox {
            font-weight: bold; border: 1px solid #a0c0a0;
            border-radius: 6px; margin-top: 6px;
            padding-top: 14px; background-color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin; left: 10px;
            padding: 0 6px; color: #2d6a2d;
        }
        QPushButton {
            background-color: #4caf50; color: white;
            border: none; border-radius: 6px;
            padding: 6px 14px; font-size: 12px; font-weight: bold;
        }
        QPushButton:hover { background-color: #45a049; }
        QPushButton:disabled { background-color: #cccccc; color: #888888; }
        QPushButton#blacklistBtn { background-color: #d32f2f; }
        QPushButton#blacklistBtn:hover { background-color: #b71c1c; }
        QProgressBar {
            border: 1px solid #a0c0a0; border-radius: 6px;
            text-align: center; height: 20px; background-color: #e0f0e0;
        }
        QProgressBar::chunk { background-color: #4caf50; border-radius: 5px; }
        QComboBox, QSlider { font-size: 12px; }
        QLabel { color: #2d2d2d; }
        QTextEdit {
            border: 1px solid #a0c0a0; border-radius: 6px;
            background-color: #fafafa; font-family: Consolas, monospace; font-size: 11px;
        }
    )"));
    m_blacklistBtn->setObjectName(QStringLiteral("blacklistBtn"));
}

// [新增] 刷新森林画布
void MainWindow::refreshGarden() {
    auto records = m_controller->getAllRecords();
    m_gardenCanvas->setRecords(records);
}

// [新增] 刷新植物下拉框
void MainWindow::refreshPlantCombo() {
    QString current = m_plantCombo->currentData().toString();
    m_plantCombo->blockSignals(true);
    m_plantCombo->clear();

    auto plants = m_controller->storeManager()->allPlants();
    for (const auto& plant : plants) {
        if (!plant.unlocked) continue;
        m_plantCombo->addItem(plant.displayName, plant.typeName);
        for (const auto& v : m_gacha->allVariants()) {
            if (v.basePlant == plant.typeName && v.unlocked) {
                QString encoded = plant.typeName + QLatin1Char('|') + v.id;
                m_plantCombo->addItem(v.icon + QStringLiteral(" ") + v.displayName, encoded);
            }
        }
    }

    int idx = m_plantCombo->findData(current);
    if (idx >= 0) m_plantCombo->setCurrentIndex(idx);

    m_plantCombo->blockSignals(false);
    onPlantTypeChanged(m_plantCombo->currentIndex());
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

void MainWindow::onFocusModeChanged(int index) {
    FocusController::FocusMode mode =
        (index == 0) ? FocusController::FocusMode::Strict
                     : FocusController::FocusMode::Gentle;
    m_controller->setFocusMode(mode);
    m_violationLabel->setVisible(mode == FocusController::FocusMode::Gentle);
    logMessage(QStringLiteral("切换至") +
               (mode == FocusController::FocusMode::Strict
                    ? QStringLiteral("严格模式")
                    : QStringLiteral("温和模式")));
}

void MainWindow::onGentleViolation(int count) {
    m_violationLabel->setText(
        QStringLiteral("⚠ 温和模式：检测到违规 %1 次").arg(count));
    m_violationLabel->show();
    logMessage(QStringLiteral("[温和模式] 违规第 ") + QString::number(count) +
               QStringLiteral(" 次"));
}

void MainWindow::onCoinsEarned(int amount, int balance) {
    m_coinLabel->setText(QStringLiteral("🪙 %1").arg(balance));
    logMessage(QStringLiteral("获得 %1 金币 (余额: %2)").arg(amount).arg(balance));
}

void MainWindow::onOpenStore() {
    // 计算每种植物使用次数（原图鉴功能合并到商店）
    auto records = m_controller->getAllRecords();
    auto plants = m_controller->storeManager()->allPlants();
    QVector<int> usageCounts(plants.size(), 0);
    for (const auto& rec : records) {
        QString type = QString::fromUtf8(rec.plantType);
        for (int i = 0; i < plants.size(); ++i) {
            if (plants[i].typeName == type) {
                usageCounts[i]++;
                break;
            }
        }
    }

    StoreDialog dialog(m_controller->storeManager(),
                       m_controller->coinManager(),
                       m_resins, m_gacha, usageCounts, this);
    connect(&dialog, &StoreDialog::plantUnlocked, this, [this]() {
        refreshPlantCombo();
        checkAchievements(false);
        logMessage(QStringLiteral("商店：新植物已解锁！"));
    });
    connect(&dialog, &StoreDialog::variantUnlocked, this, [this](const QString&) {
        checkAchievements(false);
        logMessage(QStringLiteral("抽卡：获得新异色树种！"));
    });
    dialog.exec();
    m_coinLabel->setText(
        QStringLiteral("🪙 %1").arg(m_controller->coinBalance()));
}

// [新增] 打开我的森林
void MainWindow::onOpenMyForest() {
    MyForestDialog dialog(m_controller, m_forestLayout, m_gacha, this);
    dialog.exec();
    checkAchievements(false);
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
        m_modeCombo->setEnabled(true);
        m_durationSlider->setEnabled(true);
        m_violationLabel->hide();
        break;
    case SessionState::Focusing:
        m_stateLabel->setText(QStringLiteral("🌿 专注中..."));
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        m_plantCombo->setEnabled(false);
        m_modeCombo->setEnabled(false);
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
        m_modeCombo->setEnabled(true);
        m_durationSlider->setEnabled(true);
        refreshGarden();    // [新增] 成功后刷新森林
        checkAchievements();
        break;
    case SessionState::Failed:
        m_stateLabel->setText(QStringLiteral("💀 专注失败 (违规)"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_plantCombo->setEnabled(true);
        m_modeCombo->setEnabled(true);
        m_durationSlider->setEnabled(true);
        break;
    }

    m_coinLabel->setText(QStringLiteral("🪙 %1").arg(m_controller->coinBalance()));
}

void MainWindow::onGrowthUpdated(int percent) {
    m_growthBar->setValue(percent);

    auto* plant = m_controller->currentPlant();
    if (plant) {
        // 检查是否选中了异色
        QString pt = m_session->plantType();
        QString variantIcon;
        int sep = pt.indexOf(QLatin1Char('|'));
        if (sep >= 0) {
            QString vid = pt.mid(sep + 1);
            for (const auto& v : m_gacha->allVariants()) {
                if (v.id == vid) { variantIcon = v.icon; break; }
            }
        }

        QString emoji;
        switch (plant->stage()) {
        case GrowthStage::Seed:     emoji = QStringLiteral("🌰"); break;
        case GrowthStage::Sprout:   emoji = variantIcon.isEmpty() ? QStringLiteral("🌱") : variantIcon; break;
        case GrowthStage::Growing:  emoji = variantIcon.isEmpty() ? QStringLiteral("🌿") : variantIcon; break;
        case GrowthStage::Mature:   emoji = variantIcon.isEmpty() ? QStringLiteral("🌳") : variantIcon; break;
        case GrowthStage::Withered: emoji = QStringLiteral("🍂"); break;
        }
        m_plantLabel->setText(emoji);
    }
}

void MainWindow::onForegroundProcessChanged(const QString& processName) {
    m_procLabel->setText(QStringLiteral("进程: ") + processName);
}

void MainWindow::onAddToBlacklist() {
    QString current = SystemMonitor::getForegroundProcessName();
    if (!current.isEmpty()) {
        m_monitor->addBlacklistItem(current);
        logMessage(QStringLiteral("已加入黑名单: ") + current);
    }
}

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

// ════════════════════════════════════════════════════════════
// [新增] 成就系统
// ════════════════════════════════════════════════════════════

void MainWindow::checkAchievements(bool sessionJustSucceeded) {
    auto records = m_controller->getAllRecords();

    int totalSessions = 0;
    QString lastPlantType;
    int lastDuration = 0;
    for (const auto& r : records) {
        if (r.isSuccess) {
            ++totalSessions;
            lastPlantType = QString::fromUtf8(r.plantType);
            lastDuration = r.durationSeconds;
        }
    }

    AchievementManager::Context ctx;
    ctx.totalSessions         = totalSessions;
    ctx.totalCoinsEarned      = m_controller->coinManager()->totalEarned();
    ctx.lastSessionSuccess    = sessionJustSucceeded;
    ctx.lastPlantType         = lastPlantType;
    ctx.lastSessionDuration   = lastDuration;
    ctx.lastSessionViolations = m_controller->gentleViolationCount();
    ctx.lastSessionStrict     = (m_controller->focusMode() == FocusController::FocusMode::Strict);
    ctx.unlockedPlantCount    = m_controller->storeManager()->unlockedTypeNames().size();
    ctx.forestPlacementCount  = m_forestLayout->placements().size();
    ctx.variantCount          = m_gacha->unlockedCount();

    m_achievements->checkAll(ctx);
}

void MainWindow::onAchievementUnlocked(int index, const QString& id) {
    Q_UNUSED(index)
    const auto& all = m_achievements->all();
    for (const auto& a : all) {
        if (a.id == id) {
            AchievementToast* toast = new AchievementToast(
                a.icon, a.name, a.description, a.rarity, this);
            toast->showWithFade();
            logMessage(QStringLiteral("🏆 成就解锁: %1").arg(a.name));
            break;
        }
    }
}

void MainWindow::onOpenAchievements() {
    AchievementDialog dialog(m_achievements, this);
    dialog.exec();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget* w = qobject_cast<QWidget*>(obj);
        if (!w) return QMainWindow::eventFilter(obj, event);
        QDialog* dlg = qobject_cast<QDialog*>(w->window());
        if (dlg) {
            // 发掘结果页 → 返回发掘页
            QStackedWidget* s = dlg->findChild<QStackedWidget*>();
            if (s && s->property("gachaStack").toBool() && s->currentIndex() == 1) {
                s->setCurrentIndex(0);
                return true;
            }
        }
        // 旧结果弹窗兼容
        if (w->window() && w->window()->property("resultPopup").toBool()) {
            w->window()->close();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// [新增] 异色发掘
void MainWindow::onOpenGacha() {
    CoinManager* coins = m_controller->coinManager();
    if (!m_resins) {
        QMessageBox::warning(this, QStringLiteral("系统错误"), QStringLiteral("树脂系统未初始化"));
        return;
    }

    // ── 统一对话框（QStackedWidget 翻页） ──
    QDialog* dlg = new QDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFixedSize(380, 440);
    dlg->setModal(true);

    QStackedWidget* stack = new QStackedWidget(dlg);
    stack->setProperty("gachaStack", true);

    // ── 第0页：发掘界面 ──
    QWidget* page0 = new QWidget();
    QVBoxLayout* p0lay = new QVBoxLayout(page0);
    p0lay->setAlignment(Qt::AlignCenter);

    QFrame* exCard = new QFrame(page0);
    exCard->setStyleSheet(QStringLiteral(
        "QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "  stop:0 #2d1b69, stop:1 #1a1a2e);"
        "  border: 3px solid #ffd700; border-radius: 20px; padding: 14px; }"));
    QVBoxLayout* exlay = new QVBoxLayout(exCard);
    exlay->setAlignment(Qt::AlignCenter);
    exlay->setSpacing(6);

    QLabel* exIcon = new QLabel(QStringLiteral("⛏️"), page0);
    QFont xf = exIcon->font();
    xf.setPointSize(80);
    exIcon->setFont(xf);
    exIcon->setAlignment(Qt::AlignCenter);
    exlay->addWidget(exIcon);

    QLabel* exTitle = new QLabel(QStringLiteral("异色发掘"), page0);
    exTitle->setAlignment(Qt::AlignCenter);
    exTitle->setStyleSheet(QStringLiteral("color: #ffd700; font-size: 13px; font-weight: bold;"));
    exlay->addWidget(exTitle);

    bool canAfford = coins && coins->balance() >= GachaManager::pullCost();
    QPushButton* digBtn = new QPushButton(
        QStringLiteral("💰 消费 %1 金币").arg(GachaManager::pullCost()), page0);
    digBtn->setEnabled(canAfford);
    digBtn->setFixedHeight(36);
    digBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #e67e22; color: white; border: none;"
        " border-radius: 6px; font-weight: bold; font-size: 12px; padding: 0 16px; }"
        "QPushButton:disabled { background: #555; color: #999; }"
        "QPushButton:hover:enabled { background: #d35400; }"));
    exlay->addWidget(digBtn);

    QLabel* exHint = new QLabel(QStringLiteral("Esc 取消"), page0);
    exHint->setAlignment(Qt::AlignCenter);
    exHint->setStyleSheet(QStringLiteral("color: #555; font-size: 9px;"));
    exlay->addWidget(exHint);

    p0lay->addWidget(exCard);
    stack->addWidget(page0);

    // ── 第1页：结果页 ──
    QWidget* page1 = new QWidget();
    QVBoxLayout* p1lay = new QVBoxLayout(page1);
    p1lay->setAlignment(Qt::AlignCenter);
    // 先占位，拉取后填充
    QFrame* resCard = new QFrame(page1);
    QVBoxLayout* resLay = new QVBoxLayout(resCard);
    resLay->setAlignment(Qt::AlignCenter);
    resLay->setSpacing(6);

    QLabel* resIcon = new QLabel(page1);
    QFont rif = resIcon->font();
    rif.setPointSize(96);
    resIcon->setFont(rif);
    resIcon->setAlignment(Qt::AlignCenter);
    resLay->addWidget(resIcon);

    QLabel* resName = new QLabel(page1);
    resName->setAlignment(Qt::AlignCenter);
    QFont rnf = resName->font();
    rnf.setPointSize(14);
    rnf.setBold(true);
    resName->setFont(rnf);
    resName->setStyleSheet(QStringLiteral("color: white;"));
    resLay->addWidget(resName);

    QLabel* resStatus = new QLabel(page1);
    resStatus->setAlignment(Qt::AlignCenter);
    resStatus->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: bold;"));
    resLay->addWidget(resStatus);

    p1lay->addWidget(resCard);
    stack->addWidget(page1);

    // ── 布局 ──
    QVBoxLayout* mainLay = new QVBoxLayout(dlg);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->addWidget(stack);

    bool excavated = false;
    connect(digBtn, &QPushButton::clicked, dlg, [dlg, stack, coins, this, &excavated,
                                                  resIcon, resName, resStatus, resCard]() {
        if (!coins || coins->balance() < GachaManager::pullCost()) {
            QMessageBox::warning(dlg, QStringLiteral("金币不足"),
                QStringLiteral("需要 %1 金币").arg(GachaManager::pullCost()));
            return;
        }

        auto result = m_gacha->pull(*coins, *m_resins);
        excavated = true;

        resIcon->setText(result.icon);
        resName->setText(result.displayName);

        if (result.isNew) {
            resStatus->setText(QStringLiteral("✨ 新异色获得 ✨"));
            resStatus->setStyleSheet(QStringLiteral("color: #ffd700; font-size: 11px; font-weight: bold;"));
        } else {
            resStatus->setText(QStringLiteral("重复获得，已转化为 50 树脂"));
            resStatus->setStyleSheet(QStringLiteral("color: #cd7f32; font-size: 11px; font-weight: bold;"));
        }

        resCard->setStyleSheet(QStringLiteral(
            "QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "  stop:0 #1a1a2e, stop:1 #2d1b69);"
            "  border: 3px solid #ffd700; border-radius: 20px; padding: 14px; }"));

        if (result.isNew) {
            checkAchievements(false);
            logMessage(QStringLiteral("💎 发掘到新异色: %1").arg(result.displayName));
        } else {
            logMessage(QStringLiteral("重复异色 %1，获得 50 树脂").arg(result.displayName));
        }

        m_coinLabel->setText(QStringLiteral("🪙 %1").arg(coins->balance()));
        stack->setCurrentIndex(1);
    });

    stack->setCurrentIndex(0);

    // 递归装事件过滤器，使结果页点击任意处可返回发掘页
    std::function<void(QObject*)> installRec = [&](QObject* o) {
        o->installEventFilter(this);
        for (QObject* c : o->children()) installRec(c);
    };
    installRec(dlg);

    QPoint center = QApplication::primaryScreen()->geometry().center();
    dlg->move(center.x() - 190, center.y() - 220);
    dlg->exec();

    // 关闭后释放占用的金币状态
    if (!excavated) return;
    m_coinLabel->setText(
        QStringLiteral("🪙 %1").arg(coins ? coins->balance() : 0));
}
