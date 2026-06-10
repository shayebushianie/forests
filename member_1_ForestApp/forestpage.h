#ifndef FORESTPAGE_H
#define FORESTPAGE_H

#include <QWidget>

class ForestPage : public QWidget
{
    Q_OBJECT
public:
    explicit ForestPage(QWidget *parent = nullptr);
    ~ForestPage();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    int treeCount;
};
#endif