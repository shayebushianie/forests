#ifndef FOCUSCONTROLLER_H
#define FOCUSCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <memory>

#include "FocusSession.h"
#include "AbstractPlant.h"
#include "core/FocusRecord.h"
#include "CoinManager.h"
#include "StoreManager.h"   // [新增]

class RandomAccessDatabase;
class SystemMonitor;

class FocusController : public QObject {
    Q_OBJECT

public:
    enum class FocusMode {
        Strict,
        Gentle
    };
    Q_ENUM(FocusMode)

    explicit FocusController(FocusSession* session,
                             RandomAccessDatabase* db,
                             SystemMonitor* monitor,
                             const QString& coinFilePath,
                             const QString& storeFilePath,
                             const QString& inProgressPath,
                             QObject* parent = nullptr);

    ~FocusController() override;

    AbstractPlant* currentPlant() const { return m_plant.get(); }

    bool setPlantType(const QString& typeName);

    void setFocusMode(FocusMode mode) { m_focusMode = mode; }
    FocusMode focusMode() const { return m_focusMode; }

    int gentleViolationCount() const { return m_gentleViolations; }

    CoinManager* coinManager() { return &m_coinManager; }
    int coinBalance() const { return m_coinManager.balance(); }

    // [新增] 商店访问
    StoreManager* storeManager() { return &m_storeManager; }
    QStringList unlockedPlants() const { return m_storeManager.unlockedTypeNames(); }

    // [新增] 获取所有历史记录（用于森林画布）
    std::vector<FocusRecord> getAllRecords() const;

public slots:
    void startFocus();
    void pauseFocus();
    void resumeFocus();
    void cancelFocus();
    void resetFocus();

private slots:
    void onTick();
    void onWindowViolation();

signals:
    void timeUpdated(int remainingSeconds);
    void stateChanged(SessionState newState);
    void growthUpdated(int percent);

    void gentleViolation(int count);
    void coinsEarned(int amount, int balance);

private:
    void finishFocus(bool success);

    // [新增] 成长进度持久化
    void saveGrowthProgress();
    void clearGrowthProgress();
    int  loadGrowthProgress(const QString& plantType);

    FocusSession*              m_session;
    RandomAccessDatabase*      m_db;
    SystemMonitor*             m_monitor;
    QTimer*                    m_timer;
    std::unique_ptr<AbstractPlant> m_plant;

    int  m_recordIndex;
    int  m_nextRecordId;

    FocusMode m_focusMode = FocusMode::Strict;
    int       m_gentleViolations = 0;

    CoinManager  m_coinManager;
    StoreManager m_storeManager;   // [新增]
    QString      m_inProgressPath; // [新增] 进度文件路径
};

#endif // FOCUSCONTROLLER_H
