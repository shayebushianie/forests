#include "StoreDialog.h"
#include "core/StoreManager.h"
#include "core/CoinManager.h"
#include "core/ResinManager.h"
#include "core/GachaManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <algorithm>

StoreDialog::StoreDialog(StoreManager* store,
                         CoinManager* coins,
                         ResinManager* resins,
                         GachaManager* gacha,
                         const QVector<int>& usageCounts,
                         QWidget* parent)
    : QDialog(parent)
    , m_store(store)
    , m_coins(coins)
    , m_resins(resins)
    , m_gacha(gacha)
    , m_usageCounts(usageCounts) {

    setWindowTitle(QStringLiteral("🌱 商店"));
    setFixedSize(520, 580);
    setupUI();
    refreshSeeds();
    refreshVariants();
}

void StoreDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 10, 12, 10);

    // ── 余额行 ────────────────────────────────────────────
    QHBoxLayout* balanceRow = new QHBoxLayout();
    balanceRow->setSpacing(20);

    m_coinLabel = new QLabel(this);
    QFont bf = m_coinLabel->font();
    bf.setPointSize(12);
    bf.setBold(true);
    m_coinLabel->setFont(bf);
    m_coinLabel->setStyleSheet(QStringLiteral("color: #f39c12; padding: 2px;"));
    balanceRow->addWidget(m_coinLabel);

    m_resinLabel = new QLabel(this);
    m_resinLabel->setFont(bf);
    m_resinLabel->setStyleSheet(QStringLiteral("color: #cd7f32; padding: 2px;"));
    balanceRow->addWidget(m_resinLabel);
    balanceRow->addStretch();
    mainLayout->addLayout(balanceRow);

    // ── Tab 书签 ──────────────────────────────────────────
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #ddd; border-radius: 6px;"
        "  background: #fafafa; padding: 6px; }"
        "QTabBar::tab { background: #e8e8e8; border: 1px solid #ccc;"
        "  border-bottom: none; border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px; padding: 8px 20px;"
        "  font-weight: bold; font-size: 12px; }"
        "QTabBar::tab:selected { background: white; color: #2d6a2d; }"
        "QTabBar::tab:!selected { color: #888; }"));

    m_tabWidget->addTab(createSeedTab(), QStringLiteral("🪙 金币兑换"));
    if (m_gacha) {
        m_tabWidget->addTab(createVariantTab(), QStringLiteral("🧪 树脂兑换"));
    }
    mainLayout->addWidget(m_tabWidget, 1);

    // ── 关闭 ──────────────────────────────────────────────
    QPushButton* closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #4caf50; color: white;"
        " border: none; border-radius: 6px; padding: 8px 24px; font-weight: bold;"
        " font-size: 13px; }"
        "QPushButton:hover { background: #45a049; }"));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    mainLayout->addLayout(btnRow);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//  金币兑换页（种子商店）
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

QWidget* StoreDialog::createSeedTab() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { border: none; }"));

    QWidget* container = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(container);
    scrollLayout->setSpacing(8);

    QLabel* title = new QLabel(QStringLiteral("<b>🌱 种子商店</b>  — 使用金币解锁新植物"), container);
    title->setStyleSheet(QStringLiteral("color: #666; font-size: 11px;"));
    scrollLayout->addWidget(title);

    m_seedContainer = new QWidget();
    m_seedLayout = new QVBoxLayout(m_seedContainer);
    m_seedLayout->setSpacing(8);
    scrollLayout->addWidget(m_seedContainer);
    scrollLayout->addStretch();

    scroll->setWidget(container);
    return scroll;
}

void StoreDialog::refreshSeeds() {
    QLayoutItem* item;
    while ((item = m_seedLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto plants = m_store->allPlants();
    int balance = m_coins->balance();
    m_coinLabel->setText(QStringLiteral("🪙 %1 金币").arg(balance));
    if (m_resinLabel) {
        m_resinLabel->setText(QStringLiteral("\xF0\x9F\xA7\xAA %1 树脂").arg(
            m_resins ? m_resins->balance() : 0));
    }

    QMap<QString, int> usageMap;
    for (int i = 0; i < plants.size() && i < m_usageCounts.size(); ++i) {
        usageMap[plants[i].typeName] = m_usageCounts[i];
    }

    std::sort(plants.begin(), plants.end(),
        [](const StoreManager::PlantInfo& a, const StoreManager::PlantInfo& b) {
            if (a.unlocked != b.unlocked) return a.unlocked < b.unlocked;
            return a.price < b.price;
        });

    for (const auto& plant : plants) {
        QFrame* card = new QFrame(m_seedContainer);
        card->setFrameShape(QFrame::StyledPanel);
        card->setStyleSheet(QStringLiteral(
            "QFrame { background: white; border: 2px solid #a0c0a0;"
            " border-radius: 8px; padding: 10px; }"));

        QHBoxLayout* cardRow = new QHBoxLayout(card);
        cardRow->setSpacing(16);

        QLabel* iconLabel = new QLabel(plant.icon, card);
        QFont iconFont = iconLabel->font();
        iconFont.setPointSize(30);
        iconLabel->setFont(iconFont);
        cardRow->addWidget(iconLabel);

        QVBoxLayout* infoCol = new QVBoxLayout();
        QLabel* nameLabel = new QLabel(plant.displayName, card);
        QFont nf = nameLabel->font();
        nf.setPointSize(12);
        nf.setBold(true);
        nameLabel->setFont(nf);
        infoCol->addWidget(nameLabel);

        if (plant.unlocked) {
            int count = usageMap.value(plant.typeName, 0);
            QLabel* status = new QLabel(
                QStringLiteral("✅ 已解锁 · 种植 %1 次").arg(count), card);
            status->setStyleSheet(QStringLiteral("color: #27ae60; font-weight: bold;"));
            infoCol->addWidget(status);
        } else {
            QLabel* priceLabel = new QLabel(
                QStringLiteral("🪙 %1 金币").arg(plant.price), card);
            priceLabel->setStyleSheet(QStringLiteral("color: #e67e22; font-weight: bold;"));
            infoCol->addWidget(priceLabel);
        }
        cardRow->addLayout(infoCol, 1);

        if (!plant.unlocked) {
            QPushButton* buyBtn = new QPushButton(QStringLiteral("购买"), card);
            buyBtn->setFixedWidth(70);
            buyBtn->setEnabled(balance >= plant.price);
            buyBtn->setStyleSheet(QStringLiteral(
                "QPushButton { background: #27ae60; color: white;"
                " border: none; border-radius: 4px; padding: 8px;"
                " font-weight: bold; font-size: 12px; }"
                "QPushButton:disabled { background: #ccc; }"
                "QPushButton:hover:enabled { background: #219a52; }"));

            QString typeName = plant.typeName;
            connect(buyBtn, &QPushButton::clicked, this, [this, typeName, plant]() {
                if (m_store->unlock(typeName, *m_coins)) {
                    QMessageBox::information(this,
                        QStringLiteral("购买成功"),
                        QStringLiteral("✅ 成功解锁 %1！").arg(plant.displayName));
                    emit plantUnlocked();
                    refreshSeeds();
                    refreshVariants();
                } else {
                    QMessageBox::warning(this,
                        QStringLiteral("金币不足"),
                        QStringLiteral("❌ 金币不够，继续专注赚取吧！"));
                }
            });

            cardRow->addWidget(buyBtn);
        }

        m_seedLayout->addWidget(card);
    }
    m_seedLayout->addStretch();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//  树脂兑换页（异色图鉴）
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

QWidget* StoreDialog::createVariantTab() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(10);
    layout->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout* titleRow = new QHBoxLayout();
    QLabel* title = new QLabel(QStringLiteral("<b>📖 异色图鉴</b>"), page);
    titleRow->addWidget(title);
    m_progressLabel = new QLabel(page);
    m_progressLabel->setStyleSheet(QStringLiteral("color: #888; font-size: 11px;"));
    titleRow->addWidget(m_progressLabel);
    titleRow->addStretch();

    QLabel* hint = new QLabel(QStringLiteral("消耗 200 树脂解锁"), page);
    hint->setStyleSheet(QStringLiteral("color: #cd7f32; font-size: 10px; font-weight: bold;"));
    titleRow->addWidget(hint);

    layout->addLayout(titleRow);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { border: none; }"));

    m_variantContainer = new QWidget();
    scroll->setWidget(m_variantContainer);
    layout->addWidget(scroll, 1);

    return page;
}

void StoreDialog::refreshVariants() {
    if (!m_gacha) return;

    m_coinLabel->setText(QStringLiteral("🪙 %1 金币").arg(m_coins->balance()));
    int resin = m_resins ? m_resins->balance() : 0;
    m_resinLabel->setText(QStringLiteral("🧪 %1 树脂").arg(resin));

    int unlocked = m_gacha->unlockedCount();
    int total = m_gacha->variantCount();
    m_progressLabel->setText(QStringLiteral(" %1/%2").arg(unlocked).arg(total));

    QLayout* oldLayout = m_variantContainer->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    QGridLayout* grid = new QGridLayout(m_variantContainer);
    grid->setSpacing(10);

    int n = m_gacha->variantCount();
    int cols = 3;
    for (int i = 0; i < n; ++i) {
        QWidget* card = createVariantCard(i);
        if (card) {
            grid->addWidget(card, i / cols, i % cols, Qt::AlignCenter);
        }
    }
}

QWidget* StoreDialog::createVariantCard(int index) {
    const auto& variants = m_gacha->allVariants();
    if (index >= variants.size()) return nullptr;

    const auto& v = variants[index];

    QFrame* card = new QFrame(m_variantContainer);
    card->setFixedSize(140, 170);

    bool unlocked = v.unlocked;
    QString borderColor = unlocked ? QStringLiteral("#4caf50") : QStringLiteral("#ddd");
    QString bgColor = unlocked ? QStringLiteral("#f0fff0") : QStringLiteral("#fafafa");

    card->setStyleSheet(QStringLiteral(
        "QFrame { background: %1; border: 2px solid %2;"
        " border-radius: 10px; padding: 6px; }")
        .arg(bgColor, borderColor));

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(4);

    QLabel* iconLabel = new QLabel(v.icon, card);
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(unlocked ? 32 : 24);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    QLabel* nameLabel = new QLabel(v.displayName, card);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    QFont nf = nameLabel->font();
    nf.setPointSize(9);
    nf.setBold(true);
    nameLabel->setFont(nf);
    nameLabel->setStyleSheet(QStringLiteral(
        "color: %1;").arg(unlocked ? "#2d2d2d" : "#bbb"));
    layout->addWidget(nameLabel);

    if (unlocked) {
        QLabel* ownedLabel = new QLabel(QStringLiteral("✅ 已获得"), card);
        ownedLabel->setAlignment(Qt::AlignCenter);
        ownedLabel->setStyleSheet(QStringLiteral(
            "color: #27ae60; font-size: 10px; font-weight: bold;"));
        layout->addWidget(ownedLabel);
    } else {
        QPushButton* exchBtn = new QPushButton(QStringLiteral("🧪 兑换"), card);
        exchBtn->setFixedHeight(28);
        bool canAfford = (m_resins && m_resins->balance() >= 200);
        exchBtn->setEnabled(canAfford);
        exchBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #cd7f32; color: white;"
            " border: none; border-radius: 4px; font-size: 10px; font-weight: bold; }"
            "QPushButton:disabled { background: #ddd; color: #999; }"
            "QPushButton:hover:enabled { background: #b06d2a; }"));

        QString vid = v.id;
        QString vIcon = v.icon;
        QString vName = v.displayName;
        connect(exchBtn, &QPushButton::clicked, this, [this, vid, vIcon, vName]() {
            onExchange(vid, vIcon, vName);
        });
        layout->addWidget(exchBtn);
    }

    return card;
}

void StoreDialog::onExchange(const QString& variantId, const QString& vIcon, const QString& vName) {
    if (!m_resins || m_resins->balance() < 200) {
        QMessageBox::warning(this,
            QStringLiteral("树脂不足"),
            QStringLiteral("需要 200 树脂才能兑换"));
        return;
    }

    if (m_gacha->isUnlocked(variantId)) {
        QMessageBox::information(this,
            QStringLiteral("已拥有"),
            QStringLiteral("你已经拥有该异色了"));
        return;
    }

    auto ret = QMessageBox::question(this,
        QStringLiteral("确认兑换"),
        QStringLiteral("确定要消耗 200 树脂解锁该异色吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    if (!m_resins->spend(200)) {
        QMessageBox::warning(this, QStringLiteral("失败"), QStringLiteral("树脂不足"));
        return;
    }

    if (m_gacha->unlockVariant(variantId)) {
        showPullResult(vIcon, vName, true);
        emit variantUnlocked(variantId);
        refreshSeeds();
        refreshVariants();
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//  异色获得大图鉴弹窗
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void StoreDialog::showPullResult(const QString& icon, const QString& name, bool isNew) {
    QDialog* popup = new QDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->setFixedSize(360, 420);
    popup->setModal(false);

    QVBoxLayout* layout = new QVBoxLayout(popup);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(0);

    QFrame* card = new QFrame(popup);
    card->setStyleSheet(QStringLiteral(
        "QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "  stop:0 #1a1a2e, stop:1 #2d1b69);"
        "  border: 3px solid #ffd700; border-radius: 20px; padding: 16px; }"));
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(8);

    if (isNew) {
        QLabel* sub = new QLabel(QStringLiteral("✨ 新异色获得 ✨"), popup);
        sub->setAlignment(Qt::AlignCenter);
        sub->setStyleSheet(QStringLiteral("color: #ffd700; font-size: 11px; font-weight: bold;"));
        cardLayout->addWidget(sub);
    }

    QLabel* iconLabel = new QLabel(icon, popup);
    QFont bigIcon = iconLabel->font();
    bigIcon.setPointSize(96);
    iconLabel->setFont(bigIcon);
    iconLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(iconLabel);

    QLabel* nameLabel = new QLabel(name, popup);
    nameLabel->setAlignment(Qt::AlignCenter);
    QFont nf = nameLabel->font();
    nf.setPointSize(16);
    nf.setBold(true);
    nameLabel->setFont(nf);
    nameLabel->setStyleSheet(QStringLiteral("color: white;"));
    cardLayout->addWidget(nameLabel);

    layout->addWidget(card);

    QPoint center = parentWidget()
        ? parentWidget()->mapToGlobal(parentWidget()->rect().center())
        : QApplication::primaryScreen()->geometry().center();
    popup->move(center.x() - 180, center.y() - 210);
    popup->show();

    QTimer::singleShot(2000, popup, &QDialog::close);
}
