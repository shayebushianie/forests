#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QPushButton>

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

private slots:
    void onPresetClicked();
    void onStartClicked();
    void onPauseClicked();
    void onAbandonClicked();
    void updateTimerDisplay();

private:
    QLabel *timerLabel;
    QPushButton *btnCodeRush;
    QPushButton *btnDeepRead;
    QPushButton *btnFreeStudy;
    QPushButton *btnStart;
    QPushButton *btnPause;
    QPushButton *btnAbandon;

    QTimer *countdownTimer;
    int remainingSeconds;
    bool isRunning;

    void setRemainingSeconds(int seconds);
    void stopTimer();
};

#endif // HOMEPAGE_H