#include "plant/OakTree.h"

OakTree::OakTree() {}
OakTree::~OakTree() {}

QString OakTree::getImagePath() const
{
    if (m_isWithered) {
        return ":/images/oak_withered.png";
    }
    if (m_totalMinutes >= m_growThresholds[2]) {
        return ":/images/oak_mature.png";
    }
    if (m_totalMinutes >= m_growThresholds[1]) {
        return ":/images/oak_growing.png";
    }
    if (m_totalMinutes >= m_growThresholds[0]) {
        return ":/images/oak_seedling.png";
    }
    return ":/images/oak_seed.png";
}

QString OakTree::getTypeName() const { return "OakTree"; }
QString OakTree::getDisplayName() const { return "橡树"; }

QString OakTree::getStageDescription() const
{
    if (m_isWithered) return "枯萎";
    if (m_totalMinutes >= m_growThresholds[2]) return "参天大树";
    if (m_totalMinutes >= m_growThresholds[1]) return "茁壮成长";
    if (m_totalMinutes >= m_growThresholds[0]) return "幼苗期";
    return "种子";
}