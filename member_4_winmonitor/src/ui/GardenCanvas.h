#ifndef GARDENCANVAS_H
#define GARDENCANVAS_H

#include <QWidget>
#include <vector>
#include "core/FocusRecord.h"

/**
 * @brief 森林画布组件
 *
 * 负责在历史森林视图中绘制植物。
 * 使用 QPainter 绘制不同状态、不同颜色、不同位置的植物。
 *
 * 阅读 FocusRecord 中的 isSuccess 字段决定绘制"活树"还是"枯树"。
 */
class GardenCanvas : public QWidget {
    Q_OBJECT

public:
    explicit GardenCanvas(QWidget* parent = nullptr);

    /// 设置要显示的记录列表
    void setRecords(const std::vector<FocusRecord>& records);

    /// 清空画布
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    /**
     * @brief 根据记录类型和状态绘制对应植物
     * @param painter QPainter 引用
     * @param x       绘制位置 X
     * @param y       绘制位置 Y
     * @param record  记录数据
     */
    void drawPlant(QPainter& painter, int x, int y,
                   const FocusRecord& record) const;

    std::vector<FocusRecord> m_records;
};

#endif // GARDENCANVAS_H
