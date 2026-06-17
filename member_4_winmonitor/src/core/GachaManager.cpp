#include "GachaManager.h"
#include "ResinManager.h"
#include "CoinManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QRandomGenerator>

GachaManager::GachaManager(const QString& filePath)
    : m_filePath(filePath) {

    // 12 种异色树种定义（固定顺序）
    m_variants = {
        // ── 橡树变种 ─────────────────────────────────────
        { QStringLiteral("golden_oak"),   QStringLiteral("OakTree"),
          QStringLiteral("✨ 金色橡树"),   QStringLiteral("✨"), false },
        { QStringLiteral("red_oak"),      QStringLiteral("OakTree"),
          QStringLiteral("🍁 红橡"),       QStringLiteral("🍁"), false },
        { QStringLiteral("ghost_oak"),    QStringLiteral("OakTree"),
          QStringLiteral("👻 幽灵橡树"),   QStringLiteral("👻"), false },
        // ── 玫瑰变种 ─────────────────────────────────────
        { QStringLiteral("blue_rose"),    QStringLiteral("Rose"),
          QStringLiteral("💙 蓝玫瑰"),     QStringLiteral("💙"), false },
        { QStringLiteral("black_rose"),   QStringLiteral("Rose"),
          QStringLiteral("🖤 黑玫瑰"),     QStringLiteral("🖤"), false },
        { QStringLiteral("rainbow_rose"), QStringLiteral("Rose"),
          QStringLiteral("🌈 彩虹玫瑰"),   QStringLiteral("🌈"), false },
        // ── 向日葵变种 ───────────────────────────────────
        { QStringLiteral("red_sunflower"),  QStringLiteral("Sunflower"),
          QStringLiteral("❤️ 红向日葵"),    QStringLiteral("❤️"), false },
        { QStringLiteral("white_sunflower"),QStringLiteral("Sunflower"),
          QStringLiteral("🤍 白向日葵"),    QStringLiteral("🤍"), false },
        { QStringLiteral("giant_sunflower"),QStringLiteral("Sunflower"),
          QStringLiteral("🌻 巨型向日葵"),  QStringLiteral("🌻"), false },
        // ── 松树变种 ─────────────────────────────────────
        { QStringLiteral("blue_spruce"),  QStringLiteral("PineTree"),
          QStringLiteral("💚 蓝云杉"),     QStringLiteral("💚"), false },
        { QStringLiteral("golden_pine"),  QStringLiteral("PineTree"),
          QStringLiteral("⭐ 金松"),       QStringLiteral("⭐"), false },
        { QStringLiteral("snow_pine"),    QStringLiteral("PineTree"),
          QStringLiteral("❄️ 雪松"),      QStringLiteral("❄️"), false },
    };

    load();  // restore saved unlock state
}

GachaManager::~GachaManager() { save(); }

bool GachaManager::load() {
    QFile file(m_filePath);
    if (!file.exists()) { save(); return true; }
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDataStream in(&file);
    quint32 count;
    in >> count;

    for (quint32 i = 0; i < count && i < (quint32)m_variants.size(); ++i) {
        QByteArray idBytes;
        quint8 flag;
        in >> idBytes >> flag;
        // 按 id 匹配而不是索引（灵活应对 future 增删）
        int idx = indexOf(QString::fromUtf8(idBytes));
        if (idx >= 0) m_variants[idx].unlocked = (flag != 0);
    }
    file.close();
    return true;
}

bool GachaManager::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    QDataStream out(&file);
    out << (quint32)m_variants.size();
    for (const auto& v : m_variants) {
        out << v.id.toUtf8() << (quint8)(v.unlocked ? 1 : 0);
    }
    file.close();
    return true;
}

QStringList GachaManager::unlockedVariantIds() const {
    QStringList list;
    for (const auto& v : m_variants) {
        if (v.unlocked) list << v.id;
    }
    return list;
}

int GachaManager::unlockedCount() const {
    int count = 0;
    for (const auto& v : m_variants) {
        if (v.unlocked) ++count;
    }
    return count;
}

GachaManager::PullResult GachaManager::pull(CoinManager& coins, ResinManager& resins) {
    // 扣金币
    coins.spend(pullCost());

    // 随机选择一种异色
    int idx = QRandomGenerator::global()->bounded(m_variants.size());
    const auto& chosen = m_variants[idx];

    PullResult result;
    result.variantId    = chosen.id;
    result.displayName  = chosen.displayName;
    result.icon         = chosen.icon;

    if (chosen.unlocked) {
        // 重复 → 返还树脂
        result.isNew = false;
        result.resinAwarded = duplicateResin();
        resins.add(duplicateResin());
    } else {
        // 新异色 → 解锁
        result.isNew = true;
        result.resinAwarded = 0;
        m_variants[idx].unlocked = true;
        save();
    }

    return result;
}

bool GachaManager::isUnlocked(const QString& variantId) const {
    int idx = indexOf(variantId);
    return idx >= 0 && m_variants[idx].unlocked;
}

bool GachaManager::unlockVariant(const QString& variantId) {
    int idx = indexOf(variantId);
    if (idx < 0 || m_variants[idx].unlocked) return false;
    m_variants[idx].unlocked = true;
    save();
    return true;
}

int GachaManager::indexOf(const QString& variantId) const {
    for (int i = 0; i < m_variants.size(); ++i) {
        if (m_variants[i].id == variantId) return i;
    }
    return -1;
}
