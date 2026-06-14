#ifndef FOCUSCONTROLLER_H
#define FOCUSCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include "logic/CoinManager.h"
#include "logic/AchievementEngine.h"
#include "logic/StatisticsCalculator.h"

class AbstractPlant;

/**
 * @brief 专注控制器 - 番茄钟状态机
 * @details 管理专注会话的完整生命周期，通过多态调用植物生长逻辑
 * 
 * 状态流转：
 *   IDLE ──startFocus()──> FOCUSING ──计时结束──> SUCCESS
 *                    │
 *                    └──作弊/取消──> FAILED
 * 
 * 信号（供成员1 UI 使用）：
 *   - stateChanged()   : 状态变化时通知 UI
 *   - tick()           : 每秒通知 UI 更新倒计时
 *   - plantUpdated()   : 植物生长后通知 UI 更新图片
 */
class FocusController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 专注状态枚举
     */
    enum State {
        IDLE,       ///< 空闲状态（未开始）
        FOCUSING,   ///< 专注进行中
        PAUSED,     ///< 暂停（扩展功能，暂未实现）
        SUCCESS,    ///< 专注成功完成
        FAILED      ///< 专注失败（作弊或取消）
    };
    Q_ENUM(State)

    explicit FocusController(QObject* parent = nullptr);
    ~FocusController();

    // ========== 核心接口（供成员1 UI 调用） ==========

    /**
     * @brief 开始专注
     * @param minutes 专注时长（分钟）
     * @param plant   当前选择的植物（控制器将接管其生命周期）
     */
    void startFocus(int minutes, AbstractPlant* plant);

    /**
     * @brief 取消专注（用户主动放弃）
     */
    void cancelFocus();

    /**
     * @brief 获取当前状态
     */
    State getCurrentState() const { return m_state; }

    /**
     * @brief 获取剩余秒数（用于 UI 显示）
     */
    int getRemainingSeconds() const { return m_remainingSeconds; }

    /**
     * @brief 获取当前植物
     */
    AbstractPlant* getCurrentPlant() const { return m_currentPlant; }

    // ========== 供成员4（反作弊监控）调用 ==========

    /**
     * @brief 通知控制器检测到作弊行为
     * @details 由成员4的 SystemMonitor 在检测到黑名单进程时调用
     */
    void onCheatDetected();

signals:
    /**
     * @brief 状态变化信号（供成员1 UI 响应）
     * @param newState 新状态
     */
    void stateChanged(FocusController::State newState);

    /**
     * @brief 计时器滴答信号（每秒触发）
     * @param secondsRemaining 剩余秒数
     */
    void tick(int secondsRemaining);

    /**
     * @brief 植物更新信号（生长或枯萎后触发）
     * @param imagePath 新的植物图片路径
     * @param progress  当前生长进度 0-100
     * @param stageDesc 阶段描述
     */
    void plantUpdated(const QString& imagePath, int progress, const QString& stageDesc);

private slots:
    void onTimerTick();

private:
    void setState(State newState);
    void finishSuccess();
    void finishFailed(const QString& reason);

private:
    QTimer* m_timer;           ///< 倒计时定时器
    State m_state;             ///< 当前状态
    int m_remainingSeconds;    ///< 剩余秒数
    int m_totalMinutes;        ///< 本次专注总分钟数（用于生长计算）
    AbstractPlant* m_currentPlant;  ///< 当前种植的植物（控制器拥有）
    // 新增：当前专注模式（用于金币计算）
    bool m_isGentleMode;
    int m_consecutiveDays;
};

#endif // FOCUSCONTROLLER_H