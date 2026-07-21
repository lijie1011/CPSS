#include <QtTest>
#include "datamanager.h"
#include "dynamicdata.h"

class DataManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void testSingletonInstance();
    void testOwnShipUpdate();
    void testAisTargetUpdate();
    void testDataSourcePriority();
    void testDataExpiration();
    void testValidDataRetrieval();
    void testJsonParsing();
    void testBatchUpdate();
};

void DataManagerTest::testSingletonInstance()
{
    DataManager *dm1 = DataManager::instance();
    DataManager *dm2 = DataManager::instance();

    QVERIFY(dm1 != nullptr);
    QVERIFY(dm2 != nullptr);
    QCOMPARE(dm1, dm2);
}

void DataManagerTest::testOwnShipUpdate()
{
    DataManager *dm = DataManager::instance();

    QJsonObject ownShipObj;
    ownShipObj["type"] = "ownShip";
    ownShipObj["mmsi"] = "123456789";
    ownShipObj["name"] = "TestShip";
    ownShipObj["lon"] = 121.5;
    ownShipObj["lat"] = 31.2;
    ownShipObj["heading"] = 45.0;
    ownShipObj["speed"] = 10.0;
    ownShipObj["validDuration"] = 5000;

    dm->onDataReceived(ownShipObj, Protocol_UDP);

    ShipData ship = dm->getOwnShip();
    QCOMPARE(ship.mmsi, QString("123456789"));
    QCOMPARE(ship.name, QString("TestShip"));
    QCOMPARE(ship.lon, 121.5);
    QCOMPARE(ship.lat, 31.2);
    QCOMPARE(ship.heading, 45.0);
    QCOMPARE(ship.speed, 10.0);
    QVERIFY(ship.dataStatus == DataStatus_Normal);
}

void DataManagerTest::testAisTargetUpdate()
{
    DataManager *dm = DataManager::instance();

    QJsonObject targetObj;
    targetObj["type"] = "aisTarget";
    targetObj["mmsi"] = "987654321";
    targetObj["name"] = "TargetA";
    targetObj["lon"] = 121.6;
    targetObj["lat"] = 31.3;
    targetObj["heading"] = 180.0;
    targetObj["speed"] = 5.0;
    targetObj["isDanger"] = false;
    targetObj["validDuration"] = 5000;

    dm->onDataReceived(targetObj, Protocol_UDP);

    QList<AisTarget> targets = dm->getAisTargets();
    QVERIFY(targets.size() >= 1);

    AisTarget target = targets.first();
    QCOMPARE(target.mmsi, QString("987654321"));
    QCOMPARE(target.name, QString("TargetA"));
    QCOMPARE(target.lon, 121.6);
    QCOMPARE(target.lat, 31.3);
    QVERIFY(!target.isExpired());
}

void DataManagerTest::testDataSourcePriority()
{
    DataManager *dm = DataManager::instance();

    dm->setDataSourcePriority(Protocol_UDP, 5);
    dm->setDataSourcePriority(Protocol_TCP, 10);

    QCOMPARE(dm->getDataSourcePriority(Protocol_UDP), 5);
    QCOMPARE(dm->getDataSourcePriority(Protocol_TCP), 10);

    QJsonObject lowPriorityObj;
    lowPriorityObj["type"] = "ownShip";
    lowPriorityObj["mmsi"] = "111111111";
    lowPriorityObj["name"] = "LowPriority";
    lowPriorityObj["lon"] = 100.0;
    lowPriorityObj["lat"] = 20.0;
    lowPriorityObj["validDuration"] = 5000;

    dm->onDataReceived(lowPriorityObj, Protocol_UDP);

    ShipData ship = dm->getOwnShip();
    QCOMPARE(ship.mmsi, QString("111111111"));
    QCOMPARE(ship.name, QString("LowPriority"));

    QJsonObject highPriorityObj;
    highPriorityObj["type"] = "ownShip";
    highPriorityObj["mmsi"] = "222222222";
    highPriorityObj["name"] = "HighPriority";
    highPriorityObj["lon"] = 110.0;
    highPriorityObj["lat"] = 30.0;
    highPriorityObj["validDuration"] = 5000;

    dm->onDataReceived(highPriorityObj, Protocol_TCP);

    ship = dm->getOwnShip();
    QCOMPARE(ship.mmsi, QString("222222222"));
    QCOMPARE(ship.name, QString("HighPriority"));
}

void DataManagerTest::testDataExpiration()
{
    DataManager *dm = DataManager::instance();
    dm->setDefaultValidDuration(200);

    QJsonObject targetObj;
    targetObj["type"] = "aisTarget";
    targetObj["mmsi"] = "EXP_TEST_001";
    targetObj["name"] = "ExpirationTest";
    targetObj["lon"] = 121.0;
    targetObj["lat"] = 31.0;
    targetObj["heading"] = 0.0;
    targetObj["speed"] = 0.0;
    targetObj["isDanger"] = false;

    dm->onDataReceived(targetObj, Protocol_UDP);

    QList<AisTarget> targets = dm->getAisTargets();
    bool found = false;
    for (const auto &t : targets) {
        if (t.mmsi == "EXP_TEST_001") {
            found = true;
            QVERIFY(t.dataStatus == DataStatus_Normal);
            break;
        }
    }
    QVERIFY(found);

    QTest::qWait(300);

    targets = dm->getAisTargets();
    found = false;
    for (const auto &t : targets) {
        if (t.mmsi == "EXP_TEST_001") {
            found = true;
            QVERIFY(t.dataStatus == DataStatus_Expired);
            break;
        }
    }
    QVERIFY(found);

    dm->setDefaultValidDuration(5000);
}

void DataManagerTest::testValidDataRetrieval()
{
    DataManager *dm = DataManager::instance();

    QJsonObject validObj;
    validObj["type"] = "aisTarget";
    validObj["mmsi"] = "VALID_TEST_001";
    validObj["name"] = "ValidTarget";
    validObj["lon"] = 122.0;
    validObj["lat"] = 32.0;
    validObj["validDuration"] = 10000;

    dm->onDataReceived(validObj, Protocol_UDP);

    QJsonObject expiredObj;
    expiredObj["type"] = "aisTarget";
    expiredObj["mmsi"] = "EXPIRED_TEST_001";
    expiredObj["name"] = "ExpiredTarget";
    expiredObj["lon"] = 123.0;
    expiredObj["lat"] = 33.0;
    expiredObj["validDuration"] = 100;

    dm->onDataReceived(expiredObj, Protocol_UDP);

    QTest::qWait(200);

    QList<AisTarget> allTargets = dm->getAisTargets();
    QList<AisTarget> validTargets = dm->getValidAisTargets();

    int validCount = 0;
    for (const auto &t : allTargets) {
        if (t.mmsi == "VALID_TEST_001") {
            validCount++;
        } else if (t.mmsi == "EXPIRED_TEST_001") {
            validCount++;
        }
    }
    QCOMPARE(validCount, 2);

    validCount = 0;
    for (const auto &t : validTargets) {
        if (t.mmsi == "VALID_TEST_001") {
            validCount++;
        } else if (t.mmsi == "EXPIRED_TEST_001") {
            validCount++;
        }
    }
    QCOMPARE(validCount, 1);
}

void DataManagerTest::testJsonParsing()
{
    DataManager *dm = DataManager::instance();

    QJsonObject weaponObj;
    weaponObj["type"] = "weapon";
    weaponObj["id"] = "missile_001";
    weaponObj["lon"] = 121.5;
    weaponObj["lat"] = 31.2;
    weaponObj["targetLon"] = 121.6;
    weaponObj["targetLat"] = 31.3;
    weaponObj["type"] = "missile";
    weaponObj["active"] = true;
    weaponObj["validDuration"] = 2000;

    dm->onDataReceived(weaponObj, Protocol_TCP);

    QList<WeaponData> weapons = dm->getWeapons();
    bool found = false;
    for (const auto &w : weapons) {
        if (w.id == "missile_001") {
            found = true;
            QCOMPARE(w.type, QString("missile"));
            QVERIFY(w.active);
            break;
        }
    }
    QVERIFY(found);
}

void DataManagerTest::testBatchUpdate()
{
    DataManager *dm = DataManager::instance();

    QJsonArray items;

    QJsonObject target1;
    target1["type"] = "aisTarget";
    target1["mmsi"] = "BATCH_001";
    target1["name"] = "BatchTarget1";
    target1["lon"] = 120.0;
    target1["lat"] = 30.0;
    items.append(target1);

    QJsonObject target2;
    target2["type"] = "aisTarget";
    target2["mmsi"] = "BATCH_002";
    target2["name"] = "BatchTarget2";
    target2["lon"] = 120.1;
    target2["lat"] = 30.1;
    items.append(target2);

    QJsonObject batchObj;
    batchObj["type"] = "batch";
    batchObj["items"] = items;

    dm->onDataReceived(batchObj, Protocol_UDP);

    QList<AisTarget> targets = dm->getAisTargets();
    int batchCount = 0;
    for (const auto &t : targets) {
        if (t.mmsi == "BATCH_001" || t.mmsi == "BATCH_002") {
            batchCount++;
        }
    }
    QCOMPARE(batchCount, 2);
}

QTEST_GUILESS_MAIN(DataManagerTest)

#include "tst_datamanager.moc"
