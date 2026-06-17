#ifndef FOCUSSESSION_H
#define FOCUSSESSION_H

#include <QObject>
#include <QElapsedTimer>
#include <QString>
#include "FocusRecord.h"

/**
 * @brief 专注会话的顶层状态枚举
 *
 * 状态机流转关系：
 *   Idle ──[start]──> Focusing ──[pause]──> Paused ──[resume]──> Focusing
 *                      │                                          │
 *                      ├──[succeed]──> Success                    ├──[succeed]──> Success
 *                      └──[fail]───> Failed                       └──[fail]───> Failed
 *
 * 从 Success / Failed 均可通过 [reset] 回到 Idle。
 */
enum class SessionState {
    Idle,       ///< 未开始（初始状态）
    Focusing,   ///< 专注进行中
    Paused,     ///< 已暂停
    Success,    ///< 专注成功完成
    Failed      ///< 专注失败（违规/中断）
};

/**
 * @brief 专注会话数据模型
 *
 * 维护当前专注会话的所有状态数据，包括倒计时剩余秒数、已用秒数、
 *  植物类型字符串等。被 FocusController 持有并操作。
 */
class FocusSession : public QObject {
    Q_OBJECT

public:
    /// 默认专注时长（秒）= 25 分钟
    static constexpr int DEFAULT_DURATION = 25 * 60;

    /**
     * @brief 构造函数
     * @param parent Qt 父对象
     */
    explicit FocusSession(QObject* parent = nullptr);

    // ─── 状态查询 ────────────────────────────────────────────
    SessionState state() const { return m_state; }
    int totalSeconds() const { return m_totalSeconds; }
    int remainingSeconds() const { return m_remainingSeconds; }
    int elapsedSeconds() const {
        return m_totalSeconds - m_remainingSeconds;
    }
    const QString& plantType() const { return m_plantType; }
    int recordId() const { return m_recordId; }

    // ─── 状态变更 ────────────────────────────────────────────
    void setTotalSeconds(int seconds);       // 设置专注总时长
    void setPlantType(const QString& type);  // 设置当前植物类型
    void setRecordId(int id) { m_recordId = id; }
    void setState(SessionState s) { m_state = s; emit stateChanged(m_state); }

    /**
     * @brief 消耗一秒（倒计时减一），返回是否仍有剩余时间
     * @return true=还有剩余时间, false=时间到
     */
    bool tick();

    /// 重置为 Idle 状态，清空计时数据
    void reset();

    /**
     * @brief 生成对应的 FocusRecord（用于持久化）
     * @param success 是否成功
     */
    FocusRecord toRecord(bool success) const;

signals:
    /// 状态变更通知（供 Controller / View 连接）
    void stateChanged(SessionState newState);
    /// 倒计时每秒更新
    void timeUpdated(int remainingSeconds);

private:
    SessionState m_state;          ///< 当前状态
    int m_totalSeconds;            ///< 专注总时长（秒）
    int m_remainingSeconds;        ///< 剩余秒数
    QString m_plantType;           ///< 当前种植的植物类型
    int m_recordId;                ///< 关联的文件记录 ID
    QElapsedTimer m_realTimer;     ///< 真实时钟（用于精度校准）
};

#endif // FOCUSSESSION_H
