#include <QtTest>
#include "dynamicdata.h"

class DynamicDataTest : public QObject
{
    Q_OBJECT

private slots:
    void testDataStatusValues();
    void testShipDataExpiration();
    void testAisTargetExpiration();
    void testWeaponDataExpiration();
    void testSensorDataExpiration();
    void testMarkerExpiration();
    void testProtocolTypeValues();
};

void DynamicDataTest::testDataStatusValues()
{
    QCOMPARE(DataStatus_Normal, 0);
    QCOMPARE(DataStatus_Expired, 1);
    QCOMPARE(DataStatus_Invalid, 2);
}

void DynamicDataTest::testShipDataExpiration()
{
    ShipData ship;
    ship.dataStatus = DataStatus_Normal;
    ship.validUntil = QDateTime::currentMSecsSinceEpoch() + 1000;

    QVERIFY(!ship.isExpired());

    QTest::qWait(1500);

    QVERIFY(ship.isExpired());
}

void DynamicDataTest::testAisTargetExpiration()
{
    AisTarget target;
    target.dataStatus = DataStatus_Normal;
    target.validUntil = 0;

    QVERIFY(!target.isExpired());

    target.validUntil = QDateTime::currentMSecsSinceEpoch() + 500;
    QVERIFY(!target.isExpired());

    QTest::qWait(600);

    QVERIFY(target.isExpired());

    target.dataStatus = DataStatus_Expired;
    target.validUntil = QDateTime::currentMSecsSinceEpoch() + 10000;
    QVERIFY(target.isExpired());
}

void DynamicDataTest::testWeaponDataExpiration()
{
    WeaponData weapon;
    weapon.dataStatus = DataStatus_Normal;
    weapon.validUntil = QDateTime::currentMSecsSinceEpoch() + 200;

    QVERIFY(!weapon.isExpired());

    QTest::qWait(300);

    QVERIFY(weapon.isExpired());
}

void DynamicDataTest::testSensorDataExpiration()
{
    SensorData sensor;
    sensor.dataStatus = DataStatus_Normal;
    sensor.validUntil = QDateTime::currentMSecsSinceEpoch() + 300;

    QVERIFY(!sensor.isExpired());

    QTest::qWait(400);

    QVERIFY(sensor.isExpired());
}

void DynamicDataTest::testMarkerExpiration()
{
    UserMarker marker;
    marker.dataStatus = DataStatus_Normal;
    marker.validUntil = QDateTime::currentMSecsSinceEpoch() + 400;

    QVERIFY(!marker.isExpired());

    QTest::qWait(500);

    QVERIFY(marker.isExpired());

    marker.dataStatus = DataStatus_Invalid;
    QVERIFY(!marker.isExpired());
}

void DynamicDataTest::testProtocolTypeValues()
{
    QCOMPARE(Protocol_Unknown, 0);
    QCOMPARE(Protocol_DDS, 1);
    QCOMPARE(Protocol_Redis, 2);
    QCOMPARE(Protocol_UDP, 3);
    QCOMPARE(Protocol_TCP, 4);
    QCOMPARE(Protocol_WebSocket, 5);
    QCOMPARE(Protocol_HTTP, 6);
}

QTEST_GUILESS_MAIN(DynamicDataTest)

#include "tst_dynamicdata.moc"
