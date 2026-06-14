#include "controller/FocusController.h"
#include "plant/AbstractPlant.h"
#include <QDebug>

FocusController::FocusController(QObject* parent)
    : QObject(parent)
    , m_timer(nullptr)
    , m_state(IDLE)
    , m_remainingSeconds(0)
    , m_totalMinutes(0)
    , m_currentPlant(nullptr)
    , m_isGentleMode(false)      // 新增，默认为严格模式
    , m_consecutiveDays(0)       // 新增
{
}

FocusController::~FocusController()
{
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
    }
    // 注意：m_currentPlant 由工厂创建，需要释放
    // 但可能在 UI 层被复用，这里不自动释放，交由调用方管理
}

void FocusController::startFocus(int minutes, AbstractPlant* plant)
{
    // 状态检查：只有空闲状态才能开始
    if (m_state != IDLE) {
        qDebug() << "FocusController: 无法开始专注，当前状态不是 IDLE";
        return;
    }

    if (!plant) {
        qDebug() << "FocusController: 植物对象为空";
        return;
    }

    // 保存数据
    m_totalMinutes = minutes;
    m_remainingSeconds = minutes * 60;
    m_currentPlant = plant;

    // 重置植物状态（如果是复用旧植物）
    m_currentPlant->reset();

    // 创建并启动定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FocusController::onTimerTick);
    m_timer->start(1000);  // 每秒触发一次

    // 更新状态
    setState(FOCUSING);

    // 通知 UI 初始状态
    emit plantUpdated(
        m_currentPlant->getImagePath(),
        m_currentPlant->getProgress(),
        m_currentPlant->getStageDescription()
    );

    qDebug() << "FocusController: 开始专注" << minutes << "分钟，植物：" << m_currentPlant->getDisplayName();
}

void FocusController::cancelFocus()
{
    if (m_state != FOCUSING) {
        qDebug() << "FocusController: 无法取消，当前不在专注状态";
        return;
    }

    finishFailed("用户取消");
}

void FocusController::onCheatDetected()
{
    if (m_state != FOCUSING) {
        // 不在专注状态时忽略作弊信号
        return;
    }

    finishFailed("检测到作弊行为");
}

void FocusController::onTimerTick()
{
    if (m_state != FOCUSING) {
        return;
    }

    // 剩余秒数减1
    m_remainingSeconds--;

    // 发送滴答信号（UI 更新倒计时显示）
    emit tick(m_remainingSeconds);

    // 检查是否倒计时结束
    if (m_remainingSeconds <= 0) {
        finishSuccess();
    }
}

void FocusController::finishSuccess()
{
    // 停止定时器
    if (m_timer) {
        m_timer->stop();
    }

    // 🔥 多态核心：调用植物的生长逻辑
    // 不同的植物子类有不同的生长曲线
    m_currentPlant->grow(m_totalMinutes);

    // ========== 新增：金币、成就、统计 ==========
    
    // 1. 获取连续专注天数（从成就引擎获取）
    // 注意：实际应通过 AchievementEngine 接口获取，这里简化
    int consecutiveDays = m_consecutiveDays;
    
    // 2. 增加金币
    CoinManager::getInstance().addCoins(m_totalMinutes, m_isGentleMode, consecutiveDays);
    
    // 3. 获取本次获得的金币数（用于成就记录）
    int earnedCoins = m_totalMinutes;  // 简化计算，实际应与 CoinManager 一致
    if (m_isGentleMode) earnedCoins /= 2;
    
    // 4. 更新成就系统
    AchievementEngine::getInstance().onFocusCompleted(m_totalMinutes, earnedCoins, m_currentPlant->getTypeName());
    
    // 5. 更新标签统计
    StatisticsCalculator::getInstance().addRecord(m_currentPlant->getTypeName(), m_totalMinutes, true);

    // 更新状态
    setState(SUCCESS);

    // 通知 UI 植物已更新
    emit plantUpdated(
        m_currentPlant->getImagePath(),
        m_currentPlant->getProgress(),
        m_currentPlant->getStageDescription()
    );

    qDebug() << "FocusController: 专注成功！"
             << "植物:" << m_currentPlant->getDisplayName()
             << "总专注:" << m_currentPlant->getTotalMinutes() << "分钟"
             << "阶段:" << m_currentPlant->getStageDescription();
}

void FocusController::finishFailed(const QString& reason)
{
    // 停止定时器
    if (m_timer) {
        m_timer->stop();
    }

    // 🔥 多态核心：调用植物的枯萎逻辑
    m_currentPlant->wither();

    // 更新状态
    setState(FAILED);

    // 通知 UI 植物已枯萎
    emit plantUpdated(
        m_currentPlant->getImagePath(),
        m_currentPlant->getProgress(),
        m_currentPlant->getStageDescription()
    );

    qDebug() << "FocusController: 专注失败！原因：" << reason
             << "植物:" << m_currentPlant->getDisplayName() << "已枯萎";
}

void FocusController::setState(State newState)
{
    if (m_state == newState) {
        return;
    }
    m_state = newState;
    emit stateChanged(m_state);
}