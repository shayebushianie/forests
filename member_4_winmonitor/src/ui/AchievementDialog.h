#ifndef ACHIEVEMENTDIALOG_H
#define ACHIEVEMENTDIALOG_H

#include <QDialog>

class AchievementManager;

class AchievementDialog : public QDialog {
    Q_OBJECT

public:
    explicit AchievementDialog(AchievementManager* mgr, QWidget* parent = nullptr);

private:
    void setupUI();

    AchievementManager* m_mgr;
};

#endif
