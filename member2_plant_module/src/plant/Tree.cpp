#include "plant/Tree.h"

Tree::Tree()
{
    initTreeThresholds();
}

Tree::~Tree() {}

void Tree::initTreeThresholds()
{
    // 树：60分钟幼苗，180分钟成长，360分钟成熟
    m_growThresholds[0] = 60;
    m_growThresholds[1] = 180;
    m_growThresholds[2] = 360;
}

void Tree::grow(int minutes)
{
    if (m_isWithered) return;
    m_totalMinutes += minutes;
}

void Tree::wither()
{
    m_isWithered = true;
}

int Tree::getProgress() const
{
    if (m_isWithered) return 0;
    if (m_totalMinutes >= m_growThresholds[2]) return 100;
    if (m_totalMinutes >= m_growThresholds[1]) return 66;
    if (m_totalMinutes >= m_growThresholds[0]) return 33;
    return 10;
}