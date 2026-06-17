#include "AchievementManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <algorithm>
#include <QtGlobal>

AchievementManager::AchievementManager(const QString& filePath, QObject* parent)
    : QObject(parent)
    , m_filePath(filePath) {

    setupDefs();
    load();
}

void AchievementManager::setupDefs() {
    m_infos = {
        // ── 种植类（银） ────────────────────────────────────
        { QStringLiteral("first_oak"),       QStringLiteral("橡树培育家"),
          QStringLiteral("成功种出第一棵橡树"), QStringLiteral("🌳"), QStringLiteral("silver"),  false },
        { QStringLiteral("first_rose"),      QStringLiteral("玫瑰园丁"),
          QStringLiteral("成功种出第一棵玫瑰"), QStringLiteral("🌹"), QStringLiteral("silver"),  false },
        { QStringLiteral("first_sunflower"), QStringLiteral("向日葵农夫"),
          QStringLiteral("成功种出第一棵向日葵"), QStringLiteral("🌻"), QStringLiteral("silver"),  false },
        { QStringLiteral("first_pine"),      QStringLiteral("松树造林者"),
          QStringLiteral("成功种出第一棵松树"), QStringLiteral("🌲"), QStringLiteral("silver"),  false },

        // ── 收集类（金） ────────────────────────────────────
        { QStringLiteral("full_collection"), QStringLiteral("植物学家"),
          QStringLiteral("解锁所有植物种类"), QStringLiteral("📚"), QStringLiteral("gold"),    false },

        // ── 专注类 ──────────────────────────────────────────
        { QStringLiteral("first_focus"),     QStringLiteral("初出茅庐"),
          QStringLiteral("完成第一次专注"), QStringLiteral("🎯"), QStringLiteral("common"), false },
        { QStringLiteral("dedicated"),       QStringLiteral("专注达人"),
          QStringLiteral("累计完成 10 次专注"), QStringLiteral("⭐"), QStringLiteral("common"), false },
        { QStringLiteral("master"),          QStringLiteral("专注大师"),
          QStringLiteral("累计完成 50 次专注"), QStringLiteral("👑"), QStringLiteral("gold"),   false },
        { QStringLiteral("marathon"),        QStringLiteral("马拉松专注"),
          QStringLiteral("一次专注达到 120 分钟"), QStringLiteral("⏰"), QStringLiteral("silver"),  false },
        { QStringLiteral("no_distractions"), QStringLiteral("心无旁骛"),
          QStringLiteral("严格模式下无违规完成专注"), QStringLiteral("🧘"), QStringLiteral("silver"), false },

        // ── 金币类 ──────────────────────────────────────────
        { QStringLiteral("saver"),           QStringLiteral("小有积蓄"),
          QStringLiteral("累计赚取 100 金币"), QStringLiteral("🪙"), QStringLiteral("common"), false },
        { QStringLiteral("wealthy"),         QStringLiteral("富甲一方"),
          QStringLiteral("累计赚取 1000 金币"), QStringLiteral("💰"), QStringLiteral("gold"),   false },

        // ── 森林布置类 ──────────────────────────────────────
        { QStringLiteral("decorator"),       QStringLiteral("森林装饰者"),
          QStringLiteral("在森林中摆放 5 棵树"), QStringLiteral("🌲"), QStringLiteral("common"), false },
        { QStringLiteral("architect"),       QStringLiteral("森林建筑师"),
          QStringLiteral("在森林中摆放 15 棵树"), QStringLiteral("🌳"), QStringLiteral("gold"),   false },

        // ── 异色类（14-16）──────────────────────────────────
        { QStringLiteral("first_variant"),       QStringLiteral("异色初探"),
          QStringLiteral("获得第 1 种异色树种"), QStringLiteral("✨"), QStringLiteral("silver"), false },
        { QStringLiteral("variety_collector"),   QStringLiteral("色彩收藏家"),
          QStringLiteral("获得 6 种异色树种"), QStringLiteral("🌈"), QStringLiteral("gold"),   false },
        { QStringLiteral("master_collector"),    QStringLiteral("全色彩大师"),
          QStringLiteral("集齐全部 12 种异色树种"), QStringLiteral("💎"), QStringLiteral("gold"),   false },
    };
}

bool AchievementManager::load() {
    QFile file(m_filePath);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDataStream in(&file);
    qint32 magic, count;
    in >> magic >> count;

    if (magic != 0xAC00) { file.close(); return false; }

    int n = qMin(static_cast<int>(count), static_cast<int>(m_infos.size()));
    for (int i = 0; i < n; ++i) {
        quint8 flag;
        in >> flag;
        m_infos[i].unlocked = (flag != 0);
    }
    file.close();
    return true;
}

bool AchievementManager::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    QDataStream out(&file);
    out << (qint32)0xAC00 << (qint32)m_infos.size();
    for (const auto& a : m_infos) {
        out << (quint8)(a.unlocked ? 1 : 0);
    }
    file.close();
    return true;
}

int AchievementManager::indexOf(const QString& id) const {
    for (int i = 0; i < m_infos.size(); ++i) {
        if (m_infos[i].id == id) return i;
    }
    return -1;
}

bool AchievementManager::isUnlocked(int index) const {
    if (index < 0 || index >= m_infos.size()) return false;
    return m_infos[index].unlocked;
}

QVector<int> AchievementManager::checkAll(const Context& ctx) {
    QVector<int> newlyUnlocked;

    auto check = [&](int idx, bool condition) {
        if (condition && !m_infos[idx].unlocked) {
            m_infos[idx].unlocked = true;
            newlyUnlocked.append(idx);
            emit achievementUnlocked(idx, m_infos[idx].id);
        }
    };

    // 0-3: first of type
    check(0, ctx.lastSessionSuccess && ctx.lastPlantType == QStringLiteral("OakTree"));
    check(1, ctx.lastSessionSuccess && ctx.lastPlantType == QStringLiteral("Rose"));
    check(2, ctx.lastSessionSuccess && ctx.lastPlantType == QStringLiteral("Sunflower"));
    check(3, ctx.lastSessionSuccess && ctx.lastPlantType == QStringLiteral("PineTree"));

    // 4: full collection
    check(4, ctx.unlockedPlantCount >= 4);

    // 5-7: session counts (checked only when this session succeeded)
    if (ctx.lastSessionSuccess) {
        check(5, ctx.totalSessions >= 1);   // first_focus
        check(6, ctx.totalSessions >= 10);  // dedicated
        check(7, ctx.totalSessions >= 50);  // master
    }

    // 8: marathon — 120 min
    check(8, ctx.lastSessionSuccess && ctx.lastSessionDuration >= 7200);

    // 9: no_distractions
    check(9, ctx.lastSessionSuccess && ctx.lastSessionStrict && ctx.lastSessionViolations == 0);

    // 10-11: coins
    check(10, ctx.totalCoinsEarned >= 100);
    check(11, ctx.totalCoinsEarned >= 1000);

    // 12-13: forest placement
    check(12, ctx.forestPlacementCount >= 5);
    check(13, ctx.forestPlacementCount >= 15);

    // 14-16: variant trees
    check(14, ctx.variantCount >= 1);
    check(15, ctx.variantCount >= 6);
    check(16, ctx.variantCount >= 12);

    if (!newlyUnlocked.isEmpty()) {
        save();
    }

    return newlyUnlocked;
}
