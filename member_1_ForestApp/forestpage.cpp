#include "forestpage.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QPalette>

ForestPage::ForestPage(QWidget *parent) : QWidget(parent), treeCount(12)
{
    // 设置背景为纯草绿色，完全不透明
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(200, 230, 180));
    setPalette(pal);
    // 确保大小和 stackedWidget 一致
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ForestPage::~ForestPage() {}

void ForestPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 画草地上的小点
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::darkGreen));
    for (int i = 0; i < 150; ++i) {
        int x = QRandomGenerator::global()->bounded(width());
        int y = QRandomGenerator::global()->bounded(height());
        painter.drawEllipse(x, y, 3, 3);
    }

    // 画树
    int rows = 3, cols = 4, margin = 40;
    int w = width(), h = height();
    int cellWidth = (w - 2*margin) / cols;
    int cellHeight = (h - 2*margin) / rows;
    for (int i = 0; i < treeCount && i < rows*cols; ++i) {
        int row = i / cols, col = i % cols;
        int x = margin + col * cellWidth + cellWidth/2;
        int y = margin + row * cellHeight + cellHeight/2;
        painter.setBrush(QBrush(QColor(139, 69, 19)));
        painter.drawRect(x-8, y-20, 16, 30);
        painter.setBrush(QBrush(QColor(34, 139, 34)));
        painter.drawEllipse(x-18, y-35, 36, 40);
    }

    if (treeCount == 0) {
        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, "还没有树，快去专注种树吧！");
    }
}