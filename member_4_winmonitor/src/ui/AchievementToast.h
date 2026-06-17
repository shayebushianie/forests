#ifndef ACHIEVEMENTTOAST_H
#define ACHIEVEMENTTOAST_H

#include <QFrame>
#include <QLabel>

class QPropertyAnimation;
class QParallelAnimationGroup;

class AchievementToast : public QFrame {
    Q_OBJECT

public:
    explicit AchievementToast(const QString& icon,
                               const QString& name,
                               const QString& desc,
                               const QString& rarity,
                               QWidget* parent = nullptr);

    void showWithFade();

private:
    QPropertyAnimation* m_posAnim;
    QParallelAnimationGroup* m_group;
};

#endif
