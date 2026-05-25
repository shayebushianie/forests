#include "plant/Flower.h"

Flower::Flower()
{
    initFlowerThresholds();
}

Flower::~Flower() {}

void Flower::initFlowerThresholds()
{
    // 花：20分钟幼苗，60分钟成长，120分钟成熟
    m_growThresholds[0] = 20;
    m_growThresholds[1] = 60;
    m_growThresholds[2] = 120;
}

void Flower::grow(int minutes)
{
    if (m_isWithered) return;
    m_totalMinutes += minutes;
}

void Flower::wither()
{
    m_isWithered = true;
}

int Flower::getProgress() const
{
    if (m_isWithered) return 0;
    if (m_totalMinutes >= m_growThresholds[2]) return 100;
    if (m_totalMinutes >= m_growThresholds[1]) return 60;
    if (m_totalMinutes >= m_growThresholds[0]) return 30;
    return 5;
}