#ifndef ACHIEVEMENTMANAGER_H
#define ACHIEVEMENTMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>

class AchievementManager : public QObject {
    Q_OBJECT

public:
    struct Info {
        QString id;
        QString name;
        QString description;
        QString icon;
        QString rarity;     // "gold" / "silver" / "common"
        bool unlocked = false;
    };

    struct Context {
        int     totalSessions         = 0;
        int     totalCoinsEarned      = 0;
        bool    lastSessionSuccess    = false;
        QString lastPlantType;
        int     lastSessionDuration   = 0;
        int     lastSessionViolations = 0;
        bool    lastSessionStrict     = false;
        int     unlockedPlantCount    = 0;
        int     forestPlacementCount  = 0;
        int     variantCount          = 0;
    };

    explicit AchievementManager(const QString& filePath, QObject* parent = nullptr);

    const QVector<Info>& all() const { return m_infos; }
    bool isUnlocked(int index) const;
    int count() const { return m_infos.size(); }

    QVector<int> checkAll(const Context& ctx);

signals:
    void achievementUnlocked(int index, const QString& id);

private:
    void setupDefs();
    bool load();
    bool save();

    int indexOf(const QString& id) const;

    QString m_filePath;
    QVector<Info> m_infos;
};

#endif
