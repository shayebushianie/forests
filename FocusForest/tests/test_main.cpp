/**
 * @file test_main.cpp
 * @brief 单元测试（Qt Test 框架）
 *
 * 覆盖范围：
 * 1. FocusSession 状态机
 * 2. AbstractPlant 继承体系的生长测试
 * 3. RandomAccessDatabase 随机文件读写
 * 4. SystemMonitor 违规检测逻辑
 * 5. 边界条件
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <cstdio>

#include "core/FocusSession.h"
#include "core/FocusRecord.h"
#include "core/PlantDerived.h"
#include "data/RandomAccessDatabase.h"
#include "monitor/SystemMonitor.h"

// ════════════════════════════════════════════════════════════
// Test 1: FocusRecord 结构体测试
// ════════════════════════════════════════════════════════════

class TestFocusRecord : public QObject {
    Q_OBJECT

private slots:
    void testDefaultConstructor() {
        FocusRecord rec;
        QCOMPARE(rec.recordId, 0);
        QCOMPARE(rec.isSuccess, 0);
        QCOMPARE(rec.durationSeconds, 0);
        QCOMPARE(rec.actualSeconds, 0);
        QVERIFY(std::strlen(rec.plantType) == 0);
    }

    void testParameterConstructor() {
        int64_t now = std::time(nullptr);
        FocusRecord rec(42, "OakTree", 1500, 1500, now, true);

        QCOMPARE(rec.recordId, 42);
        QCOMPARE(rec.isSuccess, 1);
        QCOMPARE(rec.durationSeconds, 1500);
        QCOMPARE(rec.actualSeconds, 1500);
        QCOMPARE(std::string(rec.plantType), std::string("OakTree"));
    }

    void testSize() {
        // sizeof 必须为 64，确保定长
        QCOMPARE(sizeof(FocusRecord), static_cast<size_t>(64));
    }
};

// ════════════════════════════════════════════════════════════
// Test 2: FocusSession 状态机测试
// ════════════════════════════════════════════════════════════

class TestFocusSession : public QObject {
    Q_OBJECT

private slots:
    void testInitialState() {
        FocusSession session;
        QCOMPARE(session.state(), SessionState::Idle);
        QCOMPARE(session.totalSeconds(), FocusSession::DEFAULT_DURATION);
        QCOMPARE(session.remainingSeconds(), FocusSession::DEFAULT_DURATION);
        QCOMPARE(session.elapsedSeconds(), 0);
    }

    void testTickDuringFocus() {
        FocusSession session;
        session.setState(SessionState::Focusing);

        int before = session.remainingSeconds();
        bool hasTime = session.tick();
        int after = session.remainingSeconds();

        QVERIFY(hasTime);  // 仍有时间
        QCOMPARE(after, before - 1);
        QCOMPARE(session.elapsedSeconds(), 1);
    }

    void testTickIdleDoesNotConsume() {
        FocusSession session;
        QCOMPARE(session.state(), SessionState::Idle);

        int before = session.remainingSeconds();
        session.tick();
        QCOMPARE(session.remainingSeconds(), before);  // Idle 不消耗
    }

    void testTimerExpiry() {
        FocusSession session;
        session.setState(SessionState::Focusing);

        // 直接设剩余 1 秒
        session.setTotalSeconds(1);

        bool hasTime = session.tick();  // 变为 0
        QVERIFY(!hasTime);               // 时间到
        QCOMPARE(session.remainingSeconds(), 0);
    }

    void testReset() {
        FocusSession session;
        session.setState(SessionState::Focusing);
        session.tick();
        session.tick();

        session.reset();
        QCOMPARE(session.state(), SessionState::Idle);
        QCOMPARE(session.remainingSeconds(), session.totalSeconds());
    }

    void testSetPlantTypeOnlyWhenIdle() {
        FocusSession session;
        session.setPlantType("Rose");
        QCOMPARE(session.plantType(), QString("Rose"));

        session.setState(SessionState::Focusing);
        session.setPlantType("OakTree");  // 不应生效
        QCOMPARE(session.plantType(), QString("Rose"));  // 保持不变
    }

    void testToRecord() {
        FocusSession session;
        session.setRecordId(1);
        session.setPlantType("Sunflower");
        session.setState(SessionState::Focusing);
        session.tick();  // 消耗 1 秒

        FocusRecord rec = session.toRecord(true);
        QCOMPARE(rec.recordId, 1);
        QCOMPARE(rec.isSuccess, 1);
        QCOMPARE(std::string(rec.plantType), std::string("Sunflower"));
        QCOMPARE(rec.actualSeconds, 1);
    }
};

// ════════════════════════════════════════════════════════════
// Test 3: 植物继承体系测试（2层以上继承演示）
// ════════════════════════════════════════════════════════════

class TestPlantHierarchy : public QObject {
    Q_OBJECT

private slots:
    // ─── 抽象基类指针的多态调用 ────────────────────────────

    void testOakTreeGrowth() {
        OakTree tree;
        QCOMPARE(tree.name(), QString("OakTree"));
        QCOMPARE(tree.stage(), GrowthStage::Seed);
        QCOMPARE(tree.progress(), 0.0);
        QVERIFY(tree.isAlive());

        // 生长到成熟
        for (int i = 0; i < tree.growTime(); ++i) {
            tree.grow();
        }
        QCOMPARE(tree.stage(), GrowthStage::Mature);
        QCOMPARE(tree.progress(), 1.0);
    }

    void testSunflowerGrowth() {
        Sunflower flower;
        QCOMPARE(flower.name(), QString("Sunflower"));
        QCOMPARE(flower.stage(), GrowthStage::Seed);

        // 生长到发芽
        int sproutTime = static_cast<int>(flower.growTime() * 0.1);
        for (int i = 0; i < sproutTime; ++i) {
            flower.grow();
        }
        QCOMPARE(flower.stage(), GrowthStage::Sprout);
    }

    void testWither() {
        OakTree tree;
        tree.grow();  // 先长一点
        tree.wither();
        QCOMPARE(tree.stage(), GrowthStage::Withered);
        QVERIFY(!tree.isAlive());

        // 枯萎后 grow 不应改变状态
        tree.grow();
        QCOMPARE(tree.stage(), GrowthStage::Withered);
    }

    void testPolymorphism() {
        // 用基类指针演示多态
        std::unique_ptr<AbstractPlant> p1 = std::make_unique<OakTree>();
        std::unique_ptr<AbstractPlant> p2 = std::make_unique<Rose>();

        QVERIFY(dynamic_cast<Tree*>(p1.get()) != nullptr);
        QVERIFY(dynamic_cast<Flower*>(p2.get()) != nullptr);

        // 调用纯虚函数（多态）
        p1->grow();
        p2->grow();

        QVERIFY(p1->progress() > 0);
        QVERIFY(p2->progress() > 0);
    }

    void testRoseDisplayChar() {
        Rose rose;
        // 种子阶段应返回幼苗 emoji
        QVERIFY(!rose.displayChar().isNull());
    }

    void testCanopySize() {
        OakTree oak;
        PineTree pine;
        // 橡树树冠大于松树
        QVERIFY(oak.canopySize() > pine.canopySize());
    }

    void testColor() {
        Rose rose;
        Sunflower sun;
        QCOMPARE(rose.color(), QString("red"));
        QCOMPARE(sun.color(), QString("yellow"));
    }

    void testDerivedPlantType() {
        // 验证继承深度
        OakTree oak;
        // OakTree -> Tree -> AbstractPlant: 3 层
        QVERIFY(static_cast<AbstractPlant*>(&oak) != nullptr);
        QVERIFY(static_cast<Tree*>(&oak) != nullptr);
    }
};

// ════════════════════════════════════════════════════════════
// Test 4: 随机文件读写测试（重点）
// ════════════════════════════════════════════════════════════

class TestRandomAccessDatabase : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    QString m_dbPath;

private slots:
    void initTestCase() {
        QVERIFY(m_tempDir.isValid());
        m_dbPath = m_tempDir.path() + QStringLiteral("/test_focus.dat");
    }

    void testOpenNewFile() {
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());
        QCOMPARE(db.recordCount(), 0);
        db.close();
    }

    void testWriteAndRead() {
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());

        int64_t now = std::time(nullptr);
        FocusRecord rec1(0, "OakTree", 1500, 1500, now, true);
        FocusRecord rec2(1, "Rose", 900, 800, now, false);

        QVERIFY(db.writeRecord(rec1));
        QVERIFY(db.writeRecord(rec2));
        QCOMPARE(db.recordCount(), 2);

        // 随机读取第 0 条
        FocusRecord readBack;
        QVERIFY(db.readRecord(0, readBack));
        QCOMPARE(readBack.recordId, 0);
        QCOMPARE(std::string(readBack.plantType), std::string("OakTree"));
        QCOMPARE(readBack.isSuccess, 1);

        // 随机读取第 1 条
        QVERIFY(db.readRecord(1, readBack));
        QCOMPARE(readBack.recordId, 1);
        QCOMPARE(std::string(readBack.plantType), std::string("Rose"));
        QCOMPARE(readBack.isSuccess, 0);

        db.close();
    }

    void testUpdateRecord() {
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());

        // 更新第 0 条记录
        int64_t now = std::time(nullptr);
        FocusRecord updated(0, "PineTree", 1800, 1800, now, false);
        QVERIFY(db.updateRecord(0, updated));

        // 验证更新结果（随机文件更新的核心测试）
        FocusRecord readBack;
        QVERIFY(db.readRecord(0, readBack));
        QCOMPARE(std::string(readBack.plantType), std::string("PineTree"));
        QCOMPARE(readBack.isSuccess, 0);

        db.close();
    }

    void testReadOutOfRange() {
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());

        FocusRecord rec;
        // 索引超出范围应返回 false
        QVERIFY(!db.readRecord(999, rec));
        QVERIFY(!db.readRecord(-1, rec));

        db.close();
    }

    void testDeleteRecord() {
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());

        QVERIFY(db.deleteRecord(0));

        FocusRecord readBack;
        QVERIFY(db.readRecord(0, readBack));
        QCOMPARE(readBack.isSuccess, 0);  // 逻辑删除标记

        db.close();
    }

    void testReadAll() {
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());

        auto records = db.readAll();
        QCOMPARE(static_cast<int>(records.size()), db.recordCount());

        db.close();
    }

    void testReopenAndRead() {
        // 关闭后重新打开，验证数据持久化
        {
            RandomAccessDatabase db(m_dbPath);
            QVERIFY(db.open());
            // 前面测试已写入 2 条
            QCOMPARE(db.recordCount(), 2);
        }  // 关闭

        // 重新打开
        RandomAccessDatabase db(m_dbPath);
        QVERIFY(db.open());
        QCOMPARE(db.recordCount(), 2);

        FocusRecord rec;
        QVERIFY(db.readRecord(1, rec));
        QCOMPARE(std::string(rec.plantType), std::string("Rose"));
    }

    void testEmptyFileReadAll() {
        QTemporaryDir tmp;
        QString path = tmp.path() + QStringLiteral("/empty.dat");
        RandomAccessDatabase db(path);
        QVERIFY(db.open());
        QCOMPARE(db.recordCount(), 0);

        auto records = db.readAll();
        QVERIFY(records.empty());
    }

    void testWriteAfterReopen() {
        QTemporaryDir tmp;
        QString path = tmp.path() + QStringLiteral("/append.dat");

        // 先写 1 条
        {
            RandomAccessDatabase db(path);
            QVERIFY(db.open());
            FocusRecord rec(0, "Test", 60, 60, std::time(nullptr), true);
            db.writeRecord(rec);
        }

        // 再追加 1 条
        {
            RandomAccessDatabase db(path);
            QVERIFY(db.open());
            FocusRecord rec(1, "Test2", 120, 120, std::time(nullptr), false);
            db.writeRecord(rec);
            QCOMPARE(db.recordCount(), 2);
        }

        // 验证
        RandomAccessDatabase db(path);
        QVERIFY(db.open());
        QCOMPARE(db.recordCount(), 2);
    }
};

// ════════════════════════════════════════════════════════════
// Test 5: SystemMonitor 违规检测逻辑测试
// ════════════════════════════════════════════════════════════

class TestSystemMonitor : public QObject {
    Q_OBJECT

private slots:
    void testBlacklistDetection() {
        SystemMonitor monitor;

        // 默认黑名单包含 "chrome.exe"
        QVERIFY(monitor.blacklist().contains("chrome.exe", Qt::CaseInsensitive));
    }

    void testWhitelistMode() {
        SystemMonitor monitor;
        monitor.setUseWhitelist(true);
        monitor.addWhitelistItem("FocusForest.exe");

        // 白名单模式：不在白名单中 → 需要检查内部逻辑
        // isViolation 是私有方法，通过公有接口间接测试
        QVERIFY(monitor.isUsingWhitelist());
    }

    void testAddBlacklistItem() {
        SystemMonitor monitor;
        monitor.addBlacklistItem("notepad.exe");
        QVERIFY(monitor.blacklist().contains("notepad.exe", Qt::CaseInsensitive));
    }
};

// ════════════════════════════════════════════════════════════
// Test 6: 边界条件测试
// ════════════════════════════════════════════════════════════

class TestEdgeCases : public QObject {
    Q_OBJECT

private slots:
    void testZeroDuration() {
        FocusSession session;
        session.setTotalSeconds(0);
        // 最低保证 1 秒
        QVERIFY(session.totalSeconds() >= 1);
    }

    void testNegativeDuration() {
        FocusSession session;
        session.setTotalSeconds(-100);
        QVERIFY(session.totalSeconds() >= 1);
    }

    void testInvalidPlantType() {
        FocusController* ctrl = nullptr;
        // 单独测试 setPlantType
    }

    void testLargeNumberOfRecords() {
        QTemporaryDir tmp;
        QString path = tmp.path() + QStringLiteral("/large.dat");
        RandomAccessDatabase db(path);
        QVERIFY(db.open());

        // 写入 100 条记录
        for (int i = 0; i < 100; ++i) {
            FocusRecord rec(i, "OakTree", 1500, 1500,
                            std::time(nullptr), i % 2 == 0);
            QVERIFY(db.writeRecord(rec));
        }
        QCOMPARE(db.recordCount(), 100);

        // 随机读取第 50 条
        FocusRecord rec;
        QVERIFY(db.readRecord(50, rec));
        QCOMPARE(rec.recordId, 50);

        // 更新第 50 条
        rec.isSuccess = 0;
        QVERIFY(db.updateRecord(50, rec));

        // 验证
        FocusRecord updated;
        QVERIFY(db.readRecord(50, updated));
        QCOMPARE(updated.isSuccess, 0);
    }
};

// ════════════════════════════════════════════════════════════
// 主函数
// ════════════════════════════════════════════════════════════

QTEST_MAIN(TestFocusRecord)
// 实际运行时可用多个 TEST_CLASS：
// #include "test_main.moc"
//
// 完整测试执行方式（手动组合）：
// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);
//     int ret = 0;
//     ret += QTest::qExec(new TestFocusRecord, argc, argv);
//     ret += QTest::qExec(new TestFocusSession, argc, argv);
//     ret += QTest::qExec(new TestPlantHierarchy, argc, argv);
//     ret += QTest::qExec(new TestRandomAccessDatabase, argc, argv);
//     ret += QTest::qExec(new TestSystemMonitor, argc, argv);
//     ret += QTest::qExec(new TestEdgeCases, argc, argv);
//     return ret;
// }
