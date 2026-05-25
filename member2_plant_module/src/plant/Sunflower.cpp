#include "plant/Sunflower.h"

Sunflower::Sunflower() {}
Sunflower::~Sunflower() {}

QString Sunflower::getImagePath() const
{
    if (m_isWithered) return ":/images/sunflower_withered.png";
    if (m_totalMinutes >= m_growThresholds[2]) return ":/images/sunflower_mature.png";
    if (m_totalMinutes >= m_growThresholds[1]) return ":/images/sunflower_growing.png";
    if (m_totalMinutes >= m_growThresholds[0]) return ":/images/sunflower_seedling.png";
    return ":/images/sunflower_seed.png";
}

QString Sunflower::getTypeName() const { return "Sunflower"; }
QString Sunflower::getDisplayName() const { return "向日葵"; }

QString Sunflower::getStageDescription() const
{
    if (m_isWithered) return "枯萎";
    if (m_totalMinutes >= m_growThresholds[2]) return "向阳盛开";
    if (m_totalMinutes >= m_growThresholds[1]) return "茁壮成长";
    if (m_totalMinutes >= m_growThresholds[0]) return "幼苗期";
    return "种子";
}