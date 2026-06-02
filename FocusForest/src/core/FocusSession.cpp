#include "FocusSession.h"
#include <algorithm>

FocusSession::FocusSession(QObject* parent)
    : QObject(parent)
    , m_state(SessionState::Idle)
    , m_totalSeconds(DEFAULT_DURATION)
    , m_remainingSeconds(DEFAULT_DURATION)
    , m_plantType(QStringLiteral("OakTree"))
    , m_recordId(0) {}

void FocusSession::setTotalSeconds(int seconds) {
    if (m_state == SessionState::Idle) {
        m_totalSeconds = std::max(1, seconds);
        m_remainingSeconds = m_totalSeconds;
    }
}

void FocusSession::setPlantType(const QString& type) {
    if (m_state == SessionState::Idle) {
        m_plantType = type;
    }
}

bool FocusSession::tick() {
    if (m_state != SessionState::Focusing) {
        return true;  // 非专注状态不消耗时间
    }
    if (m_remainingSeconds > 0) {
        --m_remainingSeconds;
        emit timeUpdated(m_remainingSeconds);
        return m_remainingSeconds > 0;
    }
    return false;  // 时间到
}

void FocusSession::reset() {
    m_state = SessionState::Idle;
    m_remainingSeconds = m_totalSeconds;
    m_recordId = 0;
    emit stateChanged(m_state);
    emit timeUpdated(m_remainingSeconds);
}

FocusRecord FocusSession::toRecord(bool success) const {
    return FocusRecord(
        m_recordId,
        m_plantType.toUtf8().constData(),
        m_totalSeconds,
        elapsedSeconds(),
        std::time(nullptr),
        success
    );
}
