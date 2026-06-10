#include "settingspage.h"
#include <QLabel>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    QLabel *label = new QLabel("⚙️ 系统设置\n\n这里以后可以配置黑白名单、专注时长等", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 20px; color: #4a6e4a; background-color: rgba(255,255,240,180); border-radius: 20px; padding: 40px;");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);
}
SettingsPage::~SettingsPage() {}