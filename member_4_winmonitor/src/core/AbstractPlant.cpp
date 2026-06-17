#include "AbstractPlant.h"
#include <algorithm>

AbstractPlant::AbstractPlant(const QString& name, int growTime)
    : m_name(name)
    , m_stage(GrowthStage::Seed)
    , m_growTime(growTime > 0 ? growTime : 1)
    , m_currentGrowth(0) {}

void AbstractPlant::setGrowth(int seconds) {
    m_currentGrowth = std::min(seconds, m_growTime);
}
