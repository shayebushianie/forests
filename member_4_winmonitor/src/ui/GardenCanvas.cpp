#include "GardenCanvas.h"
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <algorithm>
#include <cmath>

GardenCanvas::GardenCanvas(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(400, 300);
}

void GardenCanvas::setRecords(const std::vector<FocusRecord>& records) {
    m_records = records;
    update();  // 触发重绘
}

void GardenCanvas::clear() {
    m_records.clear();
    update();
}

void GardenCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 填充背景（草地渐变）
    QLinearGradient bg(0, height(), 0, 0);
    bg.setColorAt(0.0, QColor(34, 139, 34));   // forest green
    bg.setColorAt(0.6, QColor(60, 179, 113));  // medium sea green
    bg.setColorAt(1.0, QColor(135, 206, 235)); // sky blue
    painter.fillRect(rect(), bg);

    if (m_records.empty()) {
        // 空画布显示提示文字
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(16);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter,
                         QStringLiteral("还没有历史记录\n开始专注来种植你的森林吧！"));
        return;
    }

    // 在画布上布置植物（每行最多 6 棵）
    const int cols = std::min(6, static_cast<int>(m_records.size()));
    const int rows = (m_records.size() + cols - 1) / cols;
    const int spacingX = width() / (cols + 1);
    const int spacingY = height() / (rows + 1);
    const int baseY = height() - 40;  // 地面线

    for (size_t i = 0; i < m_records.size(); ++i) {
        int row = static_cast<int>(i / cols);
        int col = static_cast<int>(i % cols);
        int x = spacingX * (col + 1);
        int y = baseY - spacingY * row;
        drawPlant(painter, x, y, m_records[i]);
    }
}

void GardenCanvas::drawPlant(QPainter& painter, int x, int y,
                              const FocusRecord& record) const {
    bool alive = (record.isSuccess != 0);

    // ─── 树干（棕色矩形） ──────────────────────────────────
    QColor trunkColor = alive ? QColor(139, 69, 19) : QColor(105, 105, 105);
    painter.setBrush(trunkColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(x - 4, y - 30, 8, 30);

    // ─── 树冠（圆形） ──────────────────────────────────────
    int crownRadius = 20 + record.durationSeconds / 180;  // 时长越长树冠越大
    crownRadius = std::min(crownRadius, 40);

    QColor crownColor;
    if (alive) {
        // 根据植物类型选择颜色
        QString type = QString::fromUtf8(record.plantType);
        if (type.contains(QStringLiteral("Oak")))
            crownColor = QColor(34, 139, 34);       // 森林绿
        else if (type.contains(QStringLiteral("Pine")))
            crownColor = QColor(0, 100, 0);         // 深绿
        else if (type.contains(QStringLiteral("Rose")))
            crownColor = QColor(220, 20, 60);        // 深红
        else if (type.contains(QStringLiteral("Sunflower")))
            crownColor = QColor(255, 215, 0);        // 金黄
        else
            crownColor = QColor(50, 205, 50);         // 亮绿
    } else {
        crownColor = QColor(101, 67, 33);  // 枯萎棕色
    }

    painter.setBrush(crownColor);
    painter.drawEllipse(QPoint(x, y - 40), crownRadius, crownRadius);

    // 成功时画小亮点
    if (alive) {
        painter.setBrush(QColor(255, 255, 200, 100));
        painter.drawEllipse(QPoint(x - 5, y - 50), 4, 4);
    }
}
