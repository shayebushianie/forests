#include "plant/PineTree.h"

PineTree::PineTree() {}
PineTree::~PineTree() {}

QString PineTree::getImagePath() const
{
    if (m_isWithered) return ":/images/pine_withered.png";
    if (m_totalMinutes >= m_growThresholds[2]) return ":/images/pine_mature.png";
    if (m_totalMinutes >= m_growThresholds[1]) return ":/images/pine_growing.png";
    if (m_totalMinutes >= m_growThresholds[0]) return ":/images/pine_seedling.png";
    return ":/images/pine_seed.png";
}

QString PineTree::getTypeName() const { return "PineTree"; }
QString PineTree::getDisplayName() const { return "松树"; }

QString PineTree::getStageDescription() const
{
    if (m_isWithered) return "枯萎";
    if (m_totalMinutes >= m_growThresholds[2]) return "挺拔松柏";
    if (m_totalMinutes >= m_growThresholds[1]) return "茁壮成长";
    if (m_totalMinutes >= m_growThresholds[0]) return "幼苗期";
    return "种子";
}