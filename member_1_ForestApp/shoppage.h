#ifndef SHOPPAGE_H
#define SHOPPAGE_H

#include <QWidget>

class ShopPage : public QWidget
{
    Q_OBJECT
public:
    explicit ShopPage(QWidget *parent = nullptr);
    ~ShopPage();
};
#endif