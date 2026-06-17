#include "MyForestDialog.h"
#include "core/FocusController.h"
#include "core/StoreManager.h"
#include "core/GachaManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMap>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDateTime>
#include <QApplication>
#include <algorithm>
#include <cmath>

// ════════════════════════════════════════════════════════════
// ForestCanvas — the free-placement canvas widget
// ════════════════════════════════════════════════════════════

ForestCanvas::ForestCanvas(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(300, 250);
    setMouseTracking(true);
    setCursor(Qt::ArrowCursor);
}

void ForestCanvas::setData(const std::vector<FocusRecord>* records,
                           QVector<ForestPlacement>* placements) {
    m_records = records;
    m_placements = placements;
    update();
}

void ForestCanvas::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ── background: grass gradient ──────────────────────────
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(135, 206, 235));
    bg.setColorAt(0.45, QColor(144, 238, 144));
    bg.setColorAt(0.55, QColor(60, 179, 113));
    bg.setColorAt(1.0, QColor(34, 120, 34));
    painter.fillRect(rect(), bg);

    // ground line
    painter.setPen(QPen(QColor(34, 100, 34), 2));
    int groundY = height() * 0.55;
    painter.drawLine(0, groundY, width(), groundY);

    // ── draw all placed trees ───────────────────────────────
    if (!m_records || !m_placements) return;

    for (const auto& placement : *m_placements) {
        const FocusRecord* rec = findRecord(placement.recordId);
        if (!rec) continue;

        // highlight the tree currently being dragged
        bool isDragging = (m_dragging && m_dragRecordId == placement.recordId);
        int drawX = isDragging ? m_dragCurrentPos.x() : placement.posX;
        int drawY = isDragging ? m_dragCurrentPos.y() : placement.posY;

        drawPlant(painter, drawX, drawY, *rec);

        // show recordId label for debugging (tiny text)
        painter.setPen(QColor(255, 255, 255, 120));
        QFont f = painter.font();
        f.setPointSize(8);
        painter.setFont(f);
        painter.drawText(drawX + 12, drawY - 10,
                         QString::number(placement.recordId));
    }

    // ── hint when a tree is selected from inventory ────────
    if (m_pendingRecordId >= 0) {
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPointSize(13);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(rect().adjusted(0, 0, 0, -20),
                         Qt::AlignBottom | Qt::AlignHCenter,
                         QStringLiteral("点击画布放置树木"));
    }
}

void ForestCanvas::drawPlant(QPainter& painter, int x, int y,
                              const FocusRecord& record) const {
    bool alive = (record.isSuccess != 0);

    // ── trunk ───────────────────────────────────────────────
    QColor trunkColor = alive ? QColor(139, 69, 19) : QColor(105, 105, 105);
    painter.setBrush(trunkColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(x - 4, y - 30, 8, 30);

    // ── crown ───────────────────────────────────────────────
    int crownRadius = 20 + record.durationSeconds / 180;
    crownRadius = std::min(crownRadius, 40);

    QColor crownColor;
    if (alive) {
        // 解析异色编码
        QString type = QString::fromUtf8(record.plantType);
        int sep = type.indexOf(QLatin1Char('|'));
        QString baseType = (sep >= 0) ? type.left(sep) : type;
        QString variantId = (sep >= 0) ? type.mid(sep + 1) : QString();

        // 异色配色
        if (variantId == QStringLiteral("golden_oak") ||
            variantId == QStringLiteral("golden_pine"))
            crownColor = QColor(255, 215, 0);
        else if (variantId == QStringLiteral("red_oak") ||
                 variantId == QStringLiteral("red_sunflower") ||
                 variantId == QStringLiteral("rainbow_rose"))
            crownColor = QColor(220, 50, 50);
        else if (variantId == QStringLiteral("ghost_oak"))
            crownColor = QColor(180, 180, 220);
        else if (variantId == QStringLiteral("blue_rose") ||
                 variantId == QStringLiteral("blue_spruce"))
            crownColor = QColor(50, 100, 220);
        else if (variantId == QStringLiteral("black_rose"))
            crownColor = QColor(40, 40, 50);
        else if (variantId == QStringLiteral("white_sunflower"))
            crownColor = QColor(230, 230, 240);
        else if (variantId == QStringLiteral("giant_sunflower"))
            crownColor = QColor(200, 150, 50);
        else if (variantId == QStringLiteral("snow_pine"))
            crownColor = QColor(190, 210, 220);
        else if (baseType.contains(QStringLiteral("Oak")))
            crownColor = QColor(34, 139, 34);
        else if (baseType.contains(QStringLiteral("Pine")))
            crownColor = QColor(0, 100, 0);
        else if (baseType.contains(QStringLiteral("Rose")))
            crownColor = QColor(220, 20, 60);
        else if (baseType.contains(QStringLiteral("Sunflower")))
            crownColor = QColor(255, 215, 0);
        else
            crownColor = QColor(50, 205, 50);
    } else {
        crownColor = QColor(101, 67, 33);
    }

    painter.setBrush(crownColor);
    painter.drawEllipse(QPoint(x, y - 40), crownRadius, crownRadius);

    if (alive) {
        painter.setBrush(QColor(255, 255, 200, 100));
        painter.drawEllipse(QPoint(x - 5, y - 50), 4, 4);
    }
}

const FocusRecord* ForestCanvas::findRecord(int32_t recordId) const {
    if (!m_records) return nullptr;
    for (const auto& rec : *m_records) {
        if (rec.recordId == recordId) return &rec;
    }
    return nullptr;
}

int ForestCanvas::hitTest(QPoint pos) const {
    if (!m_placements) return -1;

    // iterate backwards so topmost (last placed) is hit first
    for (int i = m_placements->size() - 1; i >= 0; --i) {
        const auto& p = (*m_placements)[i];
        const FocusRecord* rec = findRecord(p.recordId);
        if (!rec) continue;

        int crownRadius = 20 + rec->durationSeconds / 180;
        crownRadius = std::min(crownRadius, 40);
        int cx = p.posX;
        int cy = p.posY - 40;

        int dx = pos.x() - cx;
        int dy = pos.y() - cy;
        if (dx * dx + dy * dy <= crownRadius * crownRadius) {
            return i;
        }
    }
    return -1;
}

void ForestCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        // right-click on tree → remove it
        int idx = hitTest(event->pos());
        if (idx >= 0 && m_placements) {
            int rid = (*m_placements)[idx].recordId;
            m_placements->removeAt(idx);
            emit treeRemoved(rid);
            update();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        int idx = hitTest(event->pos());

        if (idx >= 0 && m_placements) {
            // start dragging existing tree
            m_dragging = true;
            m_dragRecordId = (*m_placements)[idx].recordId;
            m_dragOffset = event->pos() - QPoint((*m_placements)[idx].posX,
                                                  (*m_placements)[idx].posY);
            m_dragCurrentPos = event->pos() - m_dragOffset;
            setCursor(Qt::ClosedHandCursor);
            return;
        }

        if (m_pendingRecordId >= 0) {
            // place new tree from inventory
            int px = event->pos().x();
            int py = std::min(event->pos().y(), height() - 20);
            m_placements->append({ m_pendingRecordId, px, py });
            int rid = m_pendingRecordId;
            m_pendingRecordId = -1;
            emit treePlaced(rid, px, py);
            update();
            return;
        }
    }
}

void ForestCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && m_placements) {
        m_dragCurrentPos = event->pos() - m_dragOffset;
        // clamp within canvas
        m_dragCurrentPos.setX(std::max(20, std::min(width() - 20, m_dragCurrentPos.x())));
        m_dragCurrentPos.setY(std::max(40, std::min(height() - 10, m_dragCurrentPos.y())));

        // update placement in-place (visual only)
        for (auto& p : *m_placements) {
            if (p.recordId == m_dragRecordId) {
                p.posX = m_dragCurrentPos.x();
                p.posY = m_dragCurrentPos.y();
                break;
            }
        }
        update();
    }
}

void ForestCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        if (m_placements) {
            for (const auto& p : *m_placements) {
                if (p.recordId == m_dragRecordId) {
                    emit treeMoved(p.recordId, p.posX, p.posY);
                    break;
                }
            }
        }
        m_dragRecordId = -1;
    }
}

// ════════════════════════════════════════════════════════════
// MyForestDialog — the placement dialog
// ════════════════════════════════════════════════════════════

MyForestDialog::MyForestDialog(FocusController* controller,
                               ForestLayoutManager* layout,
                               GachaManager* gacha,
                               QWidget* parent)
    : QDialog(parent)
    , m_controller(controller)
    , m_layout(layout)
    , m_gacha(gacha) {

    setWindowTitle(QStringLiteral("🌲 我的森林"));
    resize(780, 540);

    m_allRecords = m_controller->getAllRecords();
    m_layout->load();

    setupUI();
    populateInventory();
    m_canvas->setData(&m_allRecords, &m_layout->placements());
}

void MyForestDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    QLabel* title = new QLabel(
        QStringLiteral("<h2>🌲 我的森林</h2>"
                       "<p style='color:#666;'>自由摆放已成熟的树木，打造专属森林</p>"),
        this);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // splitter: inventory (left) | canvas (right)
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // ── left: inventory panel ───────────────────────────────
    QWidget* invPanel = new QWidget(this);
    QVBoxLayout* invLayout = new QVBoxLayout(invPanel);
    invLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* invTitle = new QLabel(QStringLiteral("📋 可用树木"), invPanel);
    QFont invFont = invTitle->font();
    invFont.setBold(true);
    invFont.setPointSize(11);
    invTitle->setFont(invFont);
    invLayout->addWidget(invTitle);

    m_inventoryList = new QListWidget(invPanel);
    m_inventoryList->setStyleSheet(QStringLiteral(
        "QListWidget { border: 1px solid #a0c0a0; border-radius: 6px;"
        " background: #fafffa; }"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #e0f0e0; }"
        "QListWidget::item:selected { background: #c8e6c9; color: #1b5e20; }"));
    connect(m_inventoryList, &QListWidget::currentRowChanged,
            this, &MyForestDialog::onInventorySelectionChanged);
    invLayout->addWidget(m_inventoryList, 1);

    QLabel* invHint = new QLabel(
        QStringLiteral("点击树木选中，再点击画布放置\n"
                       "可在画布上拖拽移动位置\n"
                       "右键点击树木移除"),
        invPanel);
    invHint->setStyleSheet(QStringLiteral("color: #888; font-size: 10px; padding: 4px;"));
    invHint->setWordWrap(true);
    invLayout->addWidget(invHint);

    splitter->addWidget(invPanel);

    // ── right: canvas ──────────────────────────────────────
    m_canvas = new ForestCanvas(this);
    m_canvas->setStyleSheet(QStringLiteral(
        "ForestCanvas { border: 2px solid #4caf50; border-radius: 8px; }"));
    connect(m_canvas, &ForestCanvas::treePlaced, this, &MyForestDialog::onTreePlaced);
    connect(m_canvas, &ForestCanvas::treeMoved,  this, &MyForestDialog::onTreeMoved);
    connect(m_canvas, &ForestCanvas::treeRemoved,this, &MyForestDialog::onTreeRemoved);

    splitter->addWidget(m_canvas);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({ 180, 560 });

    mainLayout->addWidget(splitter, 1);

    // ── bottom: hint + buttons ─────────────────────────────
    QHBoxLayout* bottomRow = new QHBoxLayout();

    m_hintLabel = new QLabel(QStringLiteral("从左侧选择树木，点击画布放置"), this);
    m_hintLabel->setStyleSheet(QStringLiteral("color: #2d6a2d; font-size: 11px;"));
    bottomRow->addWidget(m_hintLabel, 1);

    m_saveBtn = new QPushButton(QStringLiteral("💾 保存布局"), this);
    m_saveBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #2d6a2d; color: white;"
        " border: none; border-radius: 6px; padding: 8px 20px;"
        " font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #1e4f1e; }"));
    connect(m_saveBtn, &QPushButton::clicked, this, [this]() {
        m_layout->save();
        accept();
    });
    bottomRow->addWidget(m_saveBtn);

    QPushButton* closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #999; color: white;"
        " border: none; border-radius: 6px; padding: 8px 20px;"
        " font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #777; }"));
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        onSave();
        accept();
    });
    bottomRow->addWidget(closeBtn);

    mainLayout->addLayout(bottomRow);
}

void MyForestDialog::populateInventory() {
    m_inventoryList->blockSignals(true);
    m_inventoryList->clear();

    auto available = availableRecordIds();

    // store manager for display names/icons
    StoreManager* store = m_controller->storeManager();
    auto plants = store->allPlants();
    // build lookup: typeName -> PlantInfo
    QMap<QString, StoreManager::PlantInfo> plantMap;
    for (const auto& p : plants) {
        plantMap[p.typeName] = p;
    }

    if (available.isEmpty()) {
        m_inventoryList->addItem(QStringLiteral("(没有可摆放的树木)"));
        m_inventoryList->item(0)->setFlags(Qt::NoItemFlags);
    }

    for (int rid : available) {
        const FocusRecord* rec = nullptr;
        for (const auto& r : m_allRecords) {
            if (r.recordId == rid) {
                rec = &r;
                break;
            }
        }
        if (!rec) continue;

        QString type = QString::fromUtf8(rec->plantType);
        QString icon, displayName;

        // 解析异色编码 "BaseType|variantId"
        int sep = type.indexOf(QLatin1Char('|'));
        QString baseType = (sep >= 0) ? type.left(sep) : type;
        QString variantId = (sep >= 0) ? type.mid(sep + 1) : QString();

        if (!variantId.isEmpty() && m_gacha) {
            for (const auto& v : m_gacha->allVariants()) {
                if (v.id == variantId) {
                    icon = v.icon;
                    displayName = v.displayName;
                    break;
                }
            }
        }
        if (icon.isEmpty()) {
            icon = plantMap.contains(baseType) ? plantMap[baseType].icon : QStringLiteral("🌳");
            displayName = plantMap.contains(baseType) ? plantMap[baseType].displayName : baseType;
        }

        QDateTime dt = QDateTime::fromSecsSinceEpoch(rec->timestamp);
        QString dateStr = dt.toString(QStringLiteral("yyyy/MM/dd"));

        QString label = QStringLiteral("%1 %2  (%3)")
                            .arg(icon, displayName, dateStr);
        m_inventoryList->addItem(label);
    }

    m_inventoryList->blockSignals(false);
}



QVector<int> MyForestDialog::availableRecordIds() const {
    QVector<int> ids;
    for (const auto& rec : m_allRecords) {
        if (!rec.isSuccess) continue;
        if (rec.recordId <= 0) continue;
        if (m_layout->hasPlacement(rec.recordId)) continue;
        ids.append(rec.recordId);
    }
    return ids;
}

void MyForestDialog::onInventorySelectionChanged() {
    int row = m_inventoryList->currentRow();
    auto available = availableRecordIds();

    if (row >= 0 && row < available.size()) {
        int rid = available[row];
        m_canvas->setPendingRecordId(rid);
        m_canvas->refresh();
        m_hintLabel->setText(QStringLiteral("已选中树木，点击画布放置"));
    } else {
        m_canvas->clearPending();
        m_canvas->refresh();
        m_hintLabel->setText(QStringLiteral("从左侧选择树木，点击画布放置"));
    }
}

void MyForestDialog::onTreePlaced(int /*recordId*/, int /*posX*/, int /*posY*/) {
    m_canvas->clearPending();
    populateInventory();
    m_hintLabel->setText(QStringLiteral("树木已放置！可继续拖拽调整位置"));
}

void MyForestDialog::onTreeMoved(int /*recordId*/, int /*posX*/, int /*posY*/) {
    // position already updated in canvas; nothing extra needed
}

void MyForestDialog::onTreeRemoved(int /*recordId*/) {
    m_canvas->clearPending();
    populateInventory();
    m_hintLabel->setText(QStringLiteral("树木已移除"));
}

void MyForestDialog::onSave() {
    m_layout->save();
    m_hintLabel->setText(QStringLiteral("✅ 布局已保存"));
}
