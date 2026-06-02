#include "FocusController.h"
#include "PlantDerived.h"
#include "data/RandomAccessDatabase.h"
#include "monitor/SystemMonitor.h"

#include <QDebug>
#include <cassert>

FocusController::FocusController(FocusSession* session,
                                 RandomAccessDatabase* db,
                                 SystemMonitor* monitor,
                                 QObject* parent)
    : QObject(parent)
    , m_session(session)
    , m_db(db)
    , m_monitor(monitor)
    , m_timer(new QTimer(this))
    , m_plant(nullptr)
    , m_recordIndex(-1)
    , m_nextRecordId(0) {

    // 设置定时器精度为 1 秒
    m_timer->setInterval(1000);
    m_timer->setTimerType(Qt::CoarseTimer);

    // ─── 信号连接（解耦核心） ──────────────────────────────
    connect(m_timer, &QTimer::timeout, this, &FocusController::onTick);
    connect(m_monitor, &SystemMonitor::windowViolation,
            this, &FocusController::onWindowViolation);
    connect(m_session, &FocusSession::timeUpdated,
            this, &FocusController::timeUpdated);
    connect(m_session, &FocusSession::stateChanged,
            this, &FocusController::stateChanged);
}

FocusController::~FocusController() = default;

bool FocusController::setPlantType(const QString& typeName) {
    if (m_session->state() != SessionState::Idle) {
        return false;  // 专注进行中不能换植物
    }

    if (typeName == QStringLiteral("OakTree")) {
        m_plant = std::make_unique<OakTree>();
    } else if (typeName == QStringLiteral("PineTree")) {
        m_plant = std::make_unique<PineTree>();
    } else if (typeName == QStringLiteral("Rose")) {
        m_plant = std::make_unique<Rose>();
    } else if (typeName == QStringLiteral("Sunflower")) {
        m_plant = std::make_unique<Sunflower>();
    } else {
        return false;  // 未知植物类型
    }

    m_session->setPlantType(typeName);
    return true;
}

void FocusController::startFocus() {
    if (m_session->state() != SessionState::Idle) {
        return;
    }

    // 若尚未创建植物，使用默认类型
    if (!m_plant) {
        setPlantType(QStringLiteral("OakTree"));
    }

    // 从数据库中获取下一可用记录 ID
    int count = m_db->recordCount();
    m_nextRecordId = (count > 0) ? count : 0;

    m_session->setRecordId(m_nextRecordId);
    m_session->setState(SessionState::Focusing);
    m_timer->start();

    // 通知监控线程开始检测
    m_monitor->startMonitoring();

    qDebug() << "[FocusController] 专注开始 - 植物:" << m_plant->name()
             << " 时长:" << m_session->totalSeconds() << "秒";
}

void FocusController::pauseFocus() {
    if (m_session->state() != SessionState::Focusing) {
        return;
    }
    m_session->setState(SessionState::Paused);
    m_timer->stop();
    m_monitor->stopMonitoring();
    qDebug() << "[FocusController] 专注已暂停";
}

void FocusController::resumeFocus() {
    if (m_session->state() != SessionState::Paused) {
        return;
    }
    m_session->setState(SessionState::Focusing);
    m_timer->start();
    m_monitor->startMonitoring();
    qDebug() << "[FocusController] 专注已恢复";
}

void FocusController::cancelFocus() {
    if (m_session->state() != SessionState::Focusing &&
        m_session->state() != SessionState::Paused) {
        return;
    }
    m_timer->stop();
    m_monitor->stopMonitoring();
    if (m_plant) m_plant->wither();

    // 写入失败记录
    finishFocus(false);
    m_session->reset();
}

void FocusController::resetFocus() {
    m_timer->stop();
    m_monitor->stopMonitoring();
    m_session->reset();
    qDebug() << "[FocusController] 已重置";
}

void FocusController::onTick() {
    if (!m_plant) return;

    // 1. 倒计时减一秒
    bool hasTime = m_session->tick();

    // 2. 植物生长
    m_plant->grow();

    // 3. 发出进度信号
    int pct = static_cast<int>(m_plant->progress() * 100);
    emit growthUpdated(pct);

    // 4. 时间到 => 成功
    if (!hasTime) {
        finishFocus(true);
    }
}

void FocusController::onWindowViolation() {
    qDebug() << "[FocusController] 窗口违规! 专注失败.";
    m_timer->stop();
    m_monitor->stopMonitoring();
    if (m_plant) m_plant->wither();

    finishFocus(false);

    // 短暂显示失败状态后自动重置
    QTimer::singleShot(2000, this, &FocusController::resetFocus);
}

void FocusController::finishFocus(bool success) {
    // 停止所有活动
    m_timer->stop();
    m_monitor->stopMonitoring();

    // 更新状态
    SessionState finalState = success ? SessionState::Success : SessionState::Failed;
    m_session->setState(finalState);
    if (success && m_plant) {
        m_plant->grow();  // 确保最终状态为 Mature
    }

    // 持久化到二进制文件（随机文件写）
    FocusRecord record = m_session->toRecord(success);
    if (m_recordIndex >= 0) {
        m_db->updateRecord(m_recordIndex, record);
    } else {
        m_db->writeRecord(record);
        m_recordIndex = m_db->recordCount() - 1;
    }

    qDebug() << "[FocusController] 专注结束 - 结果:"
             << (success ? "成功" : "失败");
}
