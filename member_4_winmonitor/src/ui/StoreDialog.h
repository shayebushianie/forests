#ifndef STOREDIALOG_H
#define STOREDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QVector>

class StoreManager;
class CoinManager;
class ResinManager;
class GachaManager;

class StoreDialog : public QDialog {
    Q_OBJECT

public:
    explicit StoreDialog(StoreManager* store,
                         CoinManager* coins,
                         ResinManager* resins = nullptr,
                         GachaManager* gacha = nullptr,
                         const QVector<int>& usageCounts = {},
                         QWidget* parent = nullptr);

signals:
    void plantUnlocked();
    void variantUnlocked(const QString& variantId);

private:
    void setupUI();
    QWidget* createSeedTab();
    QWidget* createVariantTab();
    void refreshSeeds();
    void refreshVariants();
    QWidget* createVariantCard(int index);
    void onExchange(const QString& variantId, const QString& vIcon, const QString& vName);
    void showPullResult(const QString& icon, const QString& name, bool isNew);

    StoreManager*  m_store;
    CoinManager*   m_coins;
    ResinManager*  m_resins;
    GachaManager*  m_gacha;
    QVector<int>   m_usageCounts;

    QLabel*        m_coinLabel;
    QLabel*        m_resinLabel;
    QTabWidget*    m_tabWidget;

    // seed tab
    QWidget*       m_seedContainer;
    QVBoxLayout*   m_seedLayout;

    // variant tab
    QLabel*        m_progressLabel;
    QWidget*       m_variantContainer;
};

#endif
