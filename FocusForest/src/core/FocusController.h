#ifndef FOCUSCONTROLLER_H
#define FOCUSCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <memory>

#include "FocusSession.h"
#include "AbstractPlant.h"
#include "core/FocusRecord.h"

// Forward declarations
class RandomAccessDatabase;
class SystemMonitor;

/**
 * @brief 专注控制器 —— MVC 中的 Controller
 *
 * 职责：
 * 1. 连接 View（MainWindow）与 Model（FocusSession、Plant、Database）
 * 2. 管理 QTimer 驱动倒计时精度
 * 3. 接收 SystemMonitor 的违规信号并处理
 * 4. 协调植物生长逻辑与持久化存储
 *
 * 信号槽架构（解耦核心）：
 *   View                     Controller                 Model
 *   ┌──────┐  startFocus()   ┌──────────┐  tick()      ┌─────────────┐
 *   │      │ ──────────────> │          │ ────────────> │ FocusSession │
 *   │ Main │                 │ Focus    │  grow()       ├─────────────┤
 *   │Window│  timeUpdated()  │Controller│ ────────────> │ AbstractPlant│
 *   │      │ <────────────── │          │               ├─────────────┤
 *   └──────┘                 │          │  writeRecord()│ RandomAccess │
 *                            │          │ ────────────> │ Database     │
 *                            └──────────┘               └─────────────┘
 */
class FocusController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param session 专注会话模型
     * @param db      随机文件数据库
     * @param monitor 系统窗口监控器
     * @param parent  Qt 父对象
     */
    explicit FocusController(FocusSession* session,
                             RandomAccessDatabase* db,
                             SystemMonitor* monitor,
                             QObject* parent = nullptr);

    ~FocusController() override;

    /// 获取当前植物指针（可能为 nullptr）
    AbstractPlant* currentPlant() const { return m_plant.get(); }

    /**
     * @brief 设置当前专注使用的植物类型
     * @param typeName 植物类型名 ("OakTree", "Rose", ...)
     * @return true 如果类型合法
     */
    bool setPlantType(const QString& typeName);

public slots:
    /// 开始专注（由 View 按钮触发）
    void startFocus();
    /// 暂停专注
    void pauseFocus();
    /// 恢复专注
    void resumeFocus();
    /// 终止专注（主动取消）
    void cancelFocus();
    /// 重置到 Idle
    void resetFocus();

private slots:
    /// QTimer 驱动：每秒执行一次
    void onTick();
    /// 系统监控违规回调
    void onWindowViolation();

signals:
    /// 倒计时每秒更新（转发 Session 信号）
    void timeUpdated(int remainingSeconds);
    /// 状态变更通知
    void stateChanged(SessionState newState);
    /// 植物生长进度更新 (0~100)
    void growthUpdated(int percent);

private:
    /// 专注结束处理（成功或失败）
    void finishFocus(bool success);

    FocusSession*              m_session;
    RandomAccessDatabase*      m_db;
    SystemMonitor*             m_monitor;
    QTimer*                    m_timer;
    std::unique_ptr<AbstractPlant> m_plant;

    int  m_recordIndex;   ///< 当前记录在文件中的索引
    int  m_nextRecordId;  ///< 下一可用记录 ID
};

#endif // FOCUSCONTROLLER_H
