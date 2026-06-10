#include "shoppage.h"
#include <QLabel>
#include <QVBoxLayout>

ShopPage::ShopPage(QWidget *parent) : QWidget(parent)
{
    QLabel *label = new QLabel("🌱 植物商城\n敬请期待更多可爱的小树", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 20px; color: #4a6e4a; background-color: rgba(255,255,240,180); border-radius: 20px; padding: 40px;");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);
}
ShopPage::~ShopPage() {}