#include "AchievementToast.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>

AchievementToast::AchievementToast(const QString& icon,
                                   const QString& name,
                                   const QString& desc,
                                   const QString& rarity,
                                   QWidget* parent)
    : QFrame(parent) {

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);

    QString borderColor;
    if (rarity == QStringLiteral("gold")) {
        borderColor = QStringLiteral("#ffd700");
    } else if (rarity == QStringLiteral("silver")) {
        borderColor = QStringLiteral("#c0c0c0");
    } else {
        borderColor = QStringLiteral("#4caf50");
    }

    setStyleSheet(QStringLiteral(
        "AchievementToast {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #1a1a2e, stop:1 #16213e);"
        "  border: 2px solid %1;"
        "  border-radius: 12px;"
        "  padding: 14px;"
        "}"
        "").arg(borderColor));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(14);

    // icon
    QLabel* iconLabel = new QLabel(icon, this);
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(36);
    iconLabel->setFont(iconFont);

    QGraphicsDropShadowEffect* iconShadow = new QGraphicsDropShadowEffect(this);
    iconShadow->setBlurRadius(12);
    iconShadow->setColor(QColor(0, 0, 0, 160));
    iconShadow->setOffset(0, 2);
    iconLabel->setGraphicsEffect(iconShadow);

    layout->addWidget(iconLabel);

    // text column with dark backdrop
    QFrame* textBg = new QFrame(this);
    textBg->setStyleSheet(QStringLiteral(
        "QFrame { background: rgba(0, 0, 0, 100);"
        " border-radius: 8px; padding: 10px; }"));
    QVBoxLayout* textCol = new QVBoxLayout(textBg);
    textCol->setSpacing(3);
    textCol->setContentsMargins(8, 6, 8, 6);

    QLabel* titleLabel = new QLabel(
        QStringLiteral("🏆 成就解锁！"), textBg);
    titleLabel->setStyleSheet(QStringLiteral(
        "color: %1; font-weight: bold; font-size: 12px; letter-spacing: 1px;"
        " background: transparent;").arg(borderColor));
    textCol->addWidget(titleLabel);

    QLabel* nameLabel = new QLabel(name, textBg);
    nameLabel->setStyleSheet(QStringLiteral(
        "color: white; font-weight: bold; font-size: 16px;"
        " background: transparent;"));
    textCol->addWidget(nameLabel);

    QLabel* descLabel = new QLabel(desc, textBg);
    descLabel->setStyleSheet(QStringLiteral(
        "color: #ccc; font-size: 11px;"
        " background: transparent;"));
    descLabel->setWordWrap(true);
    textCol->addWidget(descLabel);

    layout->addWidget(textBg, 1);

    setFixedWidth(400);
    adjustSize();
    setFixedHeight(height() + 20);

    // outer glow shadow
    QGraphicsDropShadowEffect* outerShadow = new QGraphicsDropShadowEffect(this);
    outerShadow->setBlurRadius(24);
    outerShadow->setColor(QColor(0, 0, 0, 180));
    outerShadow->setOffset(0, 4);
    setGraphicsEffect(outerShadow);

    // animation
    m_posAnim = new QPropertyAnimation(this, "pos", this);
    m_posAnim->setDuration(500);
    m_posAnim->setEasingCurve(QEasingCurve::OutBack);

    m_group = new QParallelAnimationGroup(this);
    m_group->addAnimation(m_posAnim);

    connect(m_group, &QParallelAnimationGroup::finished, this, [this]() {
        QTimer::singleShot(4000, this, [this]() {
            QGraphicsOpacityEffect* e = new QGraphicsOpacityEffect(this);
            e->setOpacity(1.0);
            setGraphicsEffect(e);
            QPropertyAnimation* out = new QPropertyAnimation(e, "opacity", this);
            out->setDuration(500);
            out->setStartValue(1.0);
            out->setEndValue(0.0);
            connect(out, &QPropertyAnimation::finished, this, &QWidget::close);
            out->start(QAbstractAnimation::DeleteWhenStopped);
        });
    });
}

void AchievementToast::showWithFade() {
    QScreen* screen = QApplication::primaryScreen();
    if (!screen) { show(); return; }

    QRect geo = screen->availableGeometry();
    int targetX = geo.right() - width() - 20;
    int targetY = geo.bottom() - height() - 20;

    move(geo.right() + 50, targetY);
    show();
    raise();

    m_posAnim->setStartValue(QPoint(geo.right() + 50, targetY));
    m_posAnim->setEndValue(QPoint(targetX, targetY));

    m_group->start(QAbstractAnimation::DeleteWhenStopped);
}
