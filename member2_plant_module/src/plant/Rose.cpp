#include "plant/Rose.h"

Rose::Rose() {}
Rose::~Rose() {}

QString Rose::getImagePath() const
{
    if (m_isWithered) return ":/images/rose_withered.png";
    if (m_totalMinutes >= m_growThresholds[2]) return ":/images/rose_bloom.png";
    if (m_totalMinutes >= m_growThresholds[1]) return ":/images/rose_growing.png";
    if (m_totalMinutes >= m_growThresholds[0]) return ":/images/rose_seedling.png";
    return ":/images/rose_seed.png";
}

QString Rose::getTypeName() const { return "Rose"; }
QString Rose::getDisplayName() const { return "玫瑰"; }

QString Rose::getStageDescription() const
{
    if (m_isWithered) return "枯萎";
    if (m_totalMinutes >= m_growThresholds[2]) return "盛放";
    if (m_totalMinutes >= m_growThresholds[1]) return "含苞待放";
    if (m_totalMinutes >= m_growThresholds[0]) return "幼苗期";
    return "种子";
}