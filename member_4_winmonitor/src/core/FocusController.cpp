#include "FocusController.h"
#include "PlantDerived.h"
#include "data/RandomAccessDatabase.h"
#include "monitor/SystemMonitor.h"

#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <cassert>

FocusController::FocusController(FocusSession* session,
                                 RandomAccessDatabase* db,
                                 SystemMonitor* monitor,
                                 const QString& coinFilePath,
                                 const QString& storeFilePath,
                                 const QString& inProgressPath,
                                 QObject* parent)
    : QObject(parent)
    , m_session(session)
    , m_db(db)
    , m_monitor(monitor)
    , m_timer(new QTimer(this))
    , m_plant(nullptr)
    , m_recordIndex(-1)
    , m_nextRecordId(0)
    , m_coinManager(coinFilePath)
    , m_storeManager(storeFilePath)
    , m_inProgressPath(inProgressPath) {

    m_timer->setInterval(1000);
    m_timer->setTimerType(Qt::CoarseTimer);

    connect(m_timer, &QTimer::timeout, this, &FocusController::onTick);
    connect(m_monitor, &SystemMonitor::windowViolation,
            this, &FocusController::onWindowViolation);
    connect(m_session, &FocusSession::timeUpdated,
            this, &FocusController::timeUpdated);
    connect(m_session, &FocusSession::stateChanged,
            this, &FocusController::stateChanged);

    m_coinManager.load();
    m_storeManager.load();   // [新增]
}

FocusController::~FocusController() = default;

std::vector<FocusRecord> FocusController::getAllRecords() const {
    return m_db->readAll();
}

bool FocusController::setPlantType(const QString& typeName) {
    if (m_session->state() != SessionState::Idle) {
        return false;
    }

    // 解析 "BaseType|variantId" 编码
    QString baseType = typeName;
    QString variantId;
    int sep = typeName.indexOf(QLatin1Char('|'));
    if (sep >= 0) {
        baseType = typeName.left(sep);
        variantId = typeName.mid(sep + 1);
    }

    if (!m_storeManager.isUnlocked(baseType)) {
        qWarning() << "[FocusController] 植物未解锁:" << baseType;
        return false;
    }

    if (baseType == QStringLiteral("OakTree")) {
        m_plant = std::make_unique<OakTree>();
    } else if (baseType == QStringLiteral("PineTree")) {
        m_plant = std::make_unique<PineTree>();
    } else if (baseType == QStringLiteral("Rose")) {
        m_plant = std::make_unique<Rose>();
    } else if (baseType == QStringLiteral("Sunflower")) {
        m_plant = std::make_unique<Sunflower>();
    } else {
        return false;
    }

    // 存储完整编码（含异色信息）
    m_session->setPlantType(typeName);

    // 成长进度恢复按 baseType 匹配
    int savedGrowth = loadGrowthProgress(baseType);
    if (savedGrowth > 0) {
        m_plant->setGrowth(savedGrowth);
        qDebug() << "[FocusController] 恢复成长进度:" << baseType
                 << savedGrowth << "/" << m_plant->growTime() << "秒";
    }

    return true;
}

void FocusController::startFocus() {
    if (m_session->state() != SessionState::Idle) {
        return;
    }

    if (!m_plant) {
        setPlantType(QStringLiteral("OakTree"));
    }

    m_gentleViolations = 0;

    int count = m_db->recordCount();
    m_nextRecordId = (count > 0) ? count : 0;

    m_session->setRecordId(m_nextRecordId);
    m_session->setState(SessionState::Focusing);
    m_timer->start();

    m_monitor->startMonitoring();

    qDebug() << "[FocusController] 专注开始 - 植物:" << m_plant->name()
             << " 时长:" << m_session->totalSeconds() << "秒"
             << " 模式:" << (m_focusMode == FocusMode::Strict ? "严格" : "温和");
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

    finishFocus(false);
    m_session->reset();
}

void FocusController::resetFocus() {
    m_timer->stop();
    m_monitor->stopMonitoring();

    m_plant.reset();
    m_gentleViolations = 0;
    emit growthUpdated(0);

    m_session->reset();
    qDebug() << "[FocusController] 已重置";
}

void FocusController::onTick() {
    if (!m_plant) return;

    bool hasTime = m_session->tick();

    m_plant->grow();

    int pct = static_cast<int>(m_plant->progress() * 100);
    emit growthUpdated(pct);

    if (!hasTime) {
        finishFocus(true);
    }
}

void FocusController::onWindowViolation() {
    if (m_focusMode == FocusMode::Gentle) {
        ++m_gentleViolations;
        qDebug() << "[FocusController] [温和模式] 违规次数:" << m_gentleViolations;
        emit gentleViolation(m_gentleViolations);
        return;
    }

    qDebug() << "[FocusController] [严格模式] 窗口违规! 专注失败.";
    m_timer->stop();
    m_monitor->stopMonitoring();
    if (m_plant) m_plant->wither();

    finishFocus(false);

    QTimer::singleShot(2000, this, &FocusController::resetFocus);
}

void FocusController::finishFocus(bool success) {
    m_timer->stop();
    m_monitor->stopMonitoring();

    if (success && m_plant) {
        for (int i = 0; i < m_plant->growTime(); ++i) {
            m_plant->grow();
        }
        emit growthUpdated(100);
    }

    if (success) {
        int earned = m_coinManager.earn(
            m_session->elapsedSeconds(),
            m_focusMode == FocusMode::Gentle
        );
        emit coinsEarned(earned, m_coinManager.balance());
    }

    // [新增] 失败/取消时保留成长进度，成功时清除
    if (success) {
        clearGrowthProgress();
    } else if (m_plant) {
        saveGrowthProgress();
    }

    // [修复] 先写记录到 DB，再发 stateChanged 信号
    // 这样 checkAchievements 读 DB 时能读到最新记录
    FocusRecord record = m_session->toRecord(success);
    if (m_recordIndex >= 0) {
        m_db->updateRecord(m_recordIndex, record);
    } else {
        m_db->writeRecord(record);
        m_recordIndex = m_db->recordCount() - 1;
    }

    SessionState finalState = success ? SessionState::Success : SessionState::Failed;
    m_session->setState(finalState);

    qDebug() << "[FocusController] 专注结束 - 结果:"
             << (success ? "成功" : "失败")
             << " 模式:" << (m_focusMode == FocusMode::Strict ? "严格" : "温和")
             << " 违规次数:" << m_gentleViolations;
}

// ════════════════════════════════════════════════════════════
// [新增] 成长进度持久化
// ════════════════════════════════════════════════════════════

void FocusController::saveGrowthProgress() {
    if (!m_plant) return;

    QFile file(m_inProgressPath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QByteArray typeBytes = m_plant->name().toUtf8().leftJustified(32, '\0');
    QDataStream out(&file);
    out.writeRawData(typeBytes.constData(), 32);

    int32_t growth = static_cast<int32_t>(m_plant->currentGrowth());
    out << growth;
    file.close();

    qDebug() << "[FocusController] 已保存成长进度:" << m_plant->name() << growth;
}

void FocusController::clearGrowthProgress() {
    QFile::remove(m_inProgressPath);
    qDebug() << "[FocusController] 已清除成长进度";
}

int FocusController::loadGrowthProgress(const QString& plantType) {
    QFile file(m_inProgressPath);
    if (!file.exists()) return 0;
    if (!file.open(QIODevice::ReadOnly)) return 0;

    QDataStream in(&file);
    char rawType[32] = {};
    in.readRawData(rawType, 32);

    QString savedType = QString::fromUtf8(rawType);
    if (savedType != plantType) {
        file.close();
        return 0;
    }

    int32_t growth = 0;
    in >> growth;
    file.close();

    return (growth > 0) ? static_cast<int>(growth) : 0;
}
