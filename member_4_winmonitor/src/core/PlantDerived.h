#ifndef PLANTDERIVED_H
#define PLANTDERIVED_H

#include "AbstractPlant.h"

// ════════════════════════════════════════════════════════════
// 第二层：Tree（抽象类）
// ════════════════════════════════════════════════════════════

/**
 * @brief 树木类（继承体系第二层，仍为抽象类）
 *
 * 在 AbstractPlant 基础上增加 tree-specific 属性（树冠尺寸）。
 */
class Tree : public AbstractPlant {
public:
    Tree(const QString& name, int growTime, int canopySize);

    /** 获取树冠大小（像素/单位） */
    int canopySize() const { return m_canopySize; }

    // 仍保留 grow() / wither() 为纯虚函数，由第三层实现
    void grow() override = 0;
    void wither() override = 0;
    QChar displayChar() const override = 0;

protected:
    int m_canopySize;  ///< 树冠大小
};

// ─── 第三层：具体树木 ─────────────────────────────────────

/**
 * @brief 橡树
 *
 * 生长速度中等，树冠较大。
 * 专注成功时：从种子 → 发芽 → 成长 → 橡树
 */
class OakTree : public Tree {
public:
    OakTree();
    void grow() override;
    void wither() override;
    QChar displayChar() const override;
};

/**
 * @brief 松树
 *
 * 生长速度较慢，树冠较小但更耐寒（寓意坚韧）。
 */
class PineTree : public Tree {
public:
    PineTree();
    void grow() override;
    void wither() override;
    QChar displayChar() const override;
};

// ════════════════════════════════════════════════════════════
// 第二层：Flower（抽象类）
// ════════════════════════════════════════════════════════════

/**
 * @brief 花类（继承体系第二层，仍为抽象类）
 *
 * 在 AbstractPlant 基础上增加花瓣颜色等属性。
 */
class Flower : public AbstractPlant {
public:
    Flower(const QString& name, int growTime, const QString& color);

    const QString& color() const { return m_color; }

    void grow() override = 0;
    void wither() override = 0;
    QChar displayChar() const override = 0;

protected:
    QString m_color;  ///< 花瓣颜色描述
};

// ─── 第三层：具体花卉 ─────────────────────────────────────

/**
 * @brief 玫瑰
 *
 * 生长速度较慢，颜色为红色。
 */
class Rose : public Flower {
public:
    Rose();
    void grow() override;
    void wither() override;
    QChar displayChar() const override;
};

/**
 * @brief 向日葵
 *
 * 生长速度较快，颜色为黄色。
 */
class Sunflower : public Flower {
public:
    Sunflower();
    void grow() override;
    void wither() override;
    QChar displayChar() const override;
};

#endif // PLANTDERIVED_H
