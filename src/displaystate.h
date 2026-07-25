#ifndef DISPLAYSTATE_H
#define DISPLAYSTATE_H

#include <QString>
#include <QList>
#include <QMap>

struct ComponentState {
    QString componentId;
    QString componentName;
    QString componentType;
    double range;
    bool enabled;
};

struct PlatformDisplayState {
    QString platformId;
    QString platformName;
    bool showShip;
    bool showName;
    bool showTrack;
    bool showSensors;
    bool showWeapons;
    bool showEvents;

    QList<ComponentState> sensors;
    QList<ComponentState> weapons;

    PlatformDisplayState()
        : showShip(true), showName(true), showTrack(true),
          showSensors(true), showWeapons(true), showEvents(true) {}
};

typedef QMap<QString, PlatformDisplayState> DisplayStateMap;

#endif