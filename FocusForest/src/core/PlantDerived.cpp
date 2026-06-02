#include "PlantDerived.h"
#include <algorithm>

// ════════════════════════════════════════════════════════════
// Tree
// ════════════════════════════════════════════════════════════

Tree::Tree(const QString& name, int growTime, int canopySize)
    : AbstractPlant(name, growTime)
    , m_canopySize(canopySize) {}

// ════════════════════════════════════════════════════════════
// OakTree
// ════════════════════════════════════════════════════════════

OakTree::OakTree()
    : Tree(QStringLiteral("OakTree"), 25 * 60, 12) {}

void OakTree::grow() {
    if (m_stage == GrowthStage::Withered) return;

    m_currentGrowth = std::min(m_currentGrowth + 1, m_growTime);
    double p = progress();
    if (p >= 1.0)
        m_stage = GrowthStage::Mature;
    else if (p >= 0.6)
        m_stage = GrowthStage::Growing;
    else if (p >= 0.2)
        m_stage = GrowthStage::Sprout;
}

void OakTree::wither() {
    m_stage = GrowthStage::Withered;
}

QChar OakTree::displayChar() const {
    switch (m_stage) {
        case GrowthStage::Seed:    return QChar(0x1F331); //  seedling emoji
        case GrowthStage::Sprout:  return QChar(0x1F335); //  cactus (stand-in)
        case GrowthStage::Growing: return QChar(0x1F333); //  deciduous tree
        case GrowthStage::Mature:  return QChar(0x1F332); //  evergreen tree
        case GrowthStage::Withered:return QChar(0x2618);  //  shamrock (withered)
    }
    return QChar('?');
}

// ════════════════════════════════════════════════════════════
// PineTree
// ════════════════════════════════════════════════════════════

PineTree::PineTree()
    : Tree(QStringLiteral("PineTree"), 30 * 60, 8) {}

void PineTree::grow() {
    if (m_stage == GrowthStage::Withered) return;

    m_currentGrowth = std::min(m_currentGrowth + 1, m_growTime);
    double p = progress();
    if (p >= 1.0)
        m_stage = GrowthStage::Mature;
    else if (p >= 0.7)
        m_stage = GrowthStage::Growing;
    else if (p >= 0.3)
        m_stage = GrowthStage::Sprout;
}

void PineTree::wither() {
    m_stage = GrowthStage::Withered;
}

QChar PineTree::displayChar() const {
    switch (m_stage) {
        case GrowthStage::Seed:    return QChar(0x1F331);
        case GrowthStage::Sprout:  return QChar(0x1F335);
        case GrowthStage::Growing: return QChar(0x1F334);  //  palm tree
        case GrowthStage::Mature:  return QChar(0x1F332);  //  evergreen
        case GrowthStage::Withered:return QChar(0x2618);
    }
    return QChar('?');
}

// ════════════════════════════════════════════════════════════
// Flower
// ════════════════════════════════════════════════════════════

Flower::Flower(const QString& name, int growTime, const QString& color)
    : AbstractPlant(name, growTime)
    , m_color(color) {}

// ════════════════════════════════════════════════════════════
// Rose
// ════════════════════════════════════════════════════════════

Rose::Rose()
    : Flower(QStringLiteral("Rose"), 15 * 60, QStringLiteral("red")) {}

void Rose::grow() {
    if (m_stage == GrowthStage::Withered) return;

    m_currentGrowth = std::min(m_currentGrowth + 1, m_growTime);
    double p = progress();
    if (p >= 1.0)
        m_stage = GrowthStage::Mature;
    else if (p >= 0.5)
        m_stage = GrowthStage::Growing;
    else if (p >= 0.15)
        m_stage = GrowthStage::Sprout;
}

void Rose::wither() {
    m_stage = GrowthStage::Withered;
}

QChar Rose::displayChar() const {
    switch (m_stage) {
        case GrowthStage::Seed:    return QChar(0x1F331);
        case GrowthStage::Sprout:  return QChar(0x1F335);
        case GrowthStage::Growing: return QChar(0x1F33C);  //  blossom
        case GrowthStage::Mature:  return QChar(0x1F339);  //  rose
        case GrowthStage::Withered:return QChar(0x2618);
    }
    return QChar('?');
}

// ════════════════════════════════════════════════════════════
// Sunflower
// ════════════════════════════════════════════════════════════

Sunflower::Sunflower()
    : Flower(QStringLiteral("Sunflower"), 12 * 60, QStringLiteral("yellow")) {}

void Sunflower::grow() {
    if (m_stage == GrowthStage::Withered) return;

    m_currentGrowth = std::min(m_currentGrowth + 1, m_growTime);
    double p = progress();
    if (p >= 1.0)
        m_stage = GrowthStage::Mature;
    else if (p >= 0.45)
        m_stage = GrowthStage::Growing;
    else if (p >= 0.1)
        m_stage = GrowthStage::Sprout;
}

void Sunflower::wither() {
    m_stage = GrowthStage::Withered;
}

QChar Sunflower::displayChar() const {
    switch (m_stage) {
        case GrowthStage::Seed:    return QChar(0x1F331);
        case GrowthStage::Sprout:  return QChar(0x1F335);
        case GrowthStage::Growing: return QChar(0x1F33C);
        case GrowthStage::Mature:  return QChar(0x1F33B);  //  sunflower
        case GrowthStage::Withered:return QChar(0x2618);
    }
    return QChar('?');
}
