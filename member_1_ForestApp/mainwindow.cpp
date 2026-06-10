#include "mainwindow.h"
#include "homepage.h"
#include "forestpage.h"
#include "shoppage.h"
#include "settingspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Forest 专注森林");
    resize(900, 700);

    // 创建 stackedWidget 存放页面
    stackedWidget = new QStackedWidget(this);
    homePage = new HomePage(this);
    forestPage = new ForestPage(this);
    shopPage = new ShopPage(this);
    settingsPage = new SettingsPage(this);

    stackedWidget->addWidget(homePage);
    stackedWidget->addWidget(forestPage);
    stackedWidget->addWidget(shopPage);
    stackedWidget->addWidget(settingsPage);

    // 创建底部导航按钮
    btnHome = new QPushButton("专注主页", this);
    btnForest = new QPushButton("我的森林", this);
    btnShop = new QPushButton("植物商城", this);
    btnSettings = new QPushButton("系统设置", this);
    btnHome->setFixedHeight(50);
    btnForest->setFixedHeight(50);
    btnShop->setFixedHeight(50);
    btnSettings->setFixedHeight(50);

    // 按钮样式（圆角+悬停）
    QString btnNormalStyle =
        "QPushButton { background-color: #f0f0f0; border-radius: 25px; font-size: 16px; font-weight: bold; color: #2c3e50; padding: 10px; margin: 0px 5px; }"
        "QPushButton:hover { background-color: #e0f0e0; }"
        "QPushButton:pressed { background-color: #c0dcc0; }";
    btnHome->setStyleSheet(btnNormalStyle);
    btnForest->setStyleSheet(btnNormalStyle);
    btnShop->setStyleSheet(btnNormalStyle);
    btnSettings->setStyleSheet(btnNormalStyle);

    // 底部按钮水平布局
    QHBoxLayout *navLayout = new QHBoxLayout;
    navLayout->addWidget(btnHome);
    navLayout->addWidget(btnForest);
    navLayout->addWidget(btnShop);
    navLayout->addWidget(btnSettings);
    navLayout->setContentsMargins(10, 5, 10, 5);
    navLayout->setSpacing(10);

    // 主布局
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(stackedWidget, 1);
    mainLayout->addLayout(navLayout);
    central->setLayout(mainLayout);

    // 主窗口渐变背景
    central->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0 #f5f7fa, stop:1 #e2e8f0);"
        );
    setCentralWidget(central);

    // 连接按钮信号
    connect(btnHome, &QPushButton::clicked, this, [=]() { switchPage(0); });
    connect(btnForest, &QPushButton::clicked, this, [=]() { switchPage(1); });
    connect(btnShop, &QPushButton::clicked, this, [=]() { switchPage(2); });
    connect(btnSettings, &QPushButton::clicked, this, [=]() { switchPage(3); });

    switchPage(0);
}

MainWindow::~MainWindow() {}

void MainWindow::switchPage(int index)
{
    stackedWidget->setCurrentIndex(index);
    QString activeStyle =
        "QPushButton { background-color: #a0d6a0; border-radius: 25px; font-size: 16px; font-weight: bold; color: #1e3c2c; padding: 10px; margin: 0px 5px; }";
    QString normalStyle =
        "QPushButton { background-color: #f0f0f0; border-radius: 25px; font-size: 16px; font-weight: bold; color: #2c3e50; padding: 10px; margin: 0px 5px; }"
        "QPushButton:hover { background-color: #e0f0e0; }";
    btnHome->setStyleSheet(normalStyle);
    btnForest->setStyleSheet(normalStyle);
    btnShop->setStyleSheet(normalStyle);
    btnSettings->setStyleSheet(normalStyle);
    if (index == 0) btnHome->setStyleSheet(activeStyle);
    else if (index == 1) btnForest->setStyleSheet(activeStyle);
    else if (index == 2) btnShop->setStyleSheet(activeStyle);
    else if (index == 3) btnSettings->setStyleSheet(activeStyle);
}
