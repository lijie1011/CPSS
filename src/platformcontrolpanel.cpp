#include "platformcontrolpanel.h"

#include <QApplication>

PlatformControlPanel::PlatformControlPanel(QWidget *parent)
    : QDialog(parent),
      m_updatingCheckState(false)
{
    setWindowTitle(QString::fromLocal8Bit("Platform Control Panel"));
    setMinimumSize(500, 600);
    initUI();
}

PlatformControlPanel::~PlatformControlPanel()
{
}

void PlatformControlPanel::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QString::fromLocal8Bit("Search platforms..."));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PlatformControlPanel::onSearchTextChanged);
    mainLayout->addWidget(m_searchEdit);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &PlatformControlPanel::onTreeItemClicked);
    mainLayout->addWidget(m_treeWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_applyBtn = new QPushButton(QString::fromLocal8Bit("Apply"), this);
    connect(m_applyBtn, &QPushButton::clicked, this, &PlatformControlPanel::onApplyClicked);
    btnLayout->addWidget(m_applyBtn);

    m_resetBtn = new QPushButton(QString::fromLocal8Bit("Reset"), this);
    connect(m_resetBtn, &QPushButton::clicked, this, &PlatformControlPanel::onResetClicked);
    btnLayout->addWidget(m_resetBtn);

    m_closeBtn = new QPushButton(QString::fromLocal8Bit("Close"), this);
    connect(m_closeBtn, &QPushButton::clicked, this, &PlatformControlPanel::close);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
}

void PlatformControlPanel::initWithData(const DynamicObjects &data, const DisplayStateMap &currentStates)
{
    m_currentData = data;
    populateTree(data, currentStates);
}

void PlatformControlPanel::populateTree(const DynamicObjects &data, const DisplayStateMap &currentStates)
{
    m_treeWidget->clear();

    QMap<CampType, QList<PlatformData>> groups;
    for (const auto &platform : data.platforms.values()) {
        if (!platform.isExpired()) {
            groups[platform.camp].append(platform);
        }
    }

    QList<CampType> campOrder;
    campOrder << Camp_Friendly << Camp_Enemy << Camp_Neutral;

    for (CampType camp : campOrder) {
        if (!groups.contains(camp)) continue;

        QString campName = campToString(camp);
        QTreeWidgetItem *campItem = new QTreeWidgetItem(m_treeWidget);
        campItem->setText(0, QString("%1 (%2)").arg(campName).arg(groups[camp].size()));
        campItem->setFlags(campItem->flags() | Qt::ItemIsUserCheckable);
        campItem->setCheckState(0, Qt::Checked);

        for (const PlatformData &platform : groups[camp]) {
            auto stateIt = currentStates.find(platform.id);
            PlatformDisplayState state;
            if (stateIt != currentStates.end()) {
                state = stateIt.value();
            } else {
                state.showShip = true;
                state.showName = true;
                state.showTrack = true;
                state.showSensors = true;
                state.showWeapons = true;
                state.showEvents = true;
            }
            addPlatformNode(campItem, platform, state);
        }
    }

    m_treeWidget->expandAll();
}

void PlatformControlPanel::addPlatformNode(QTreeWidgetItem *campItem, const PlatformData &platform, const PlatformDisplayState &state)
{
    QTreeWidgetItem *platformItem = new QTreeWidgetItem(campItem);
    platformItem->setText(0, QString("%1 (%2)").arg(platform.id).arg(platform.name));
    platformItem->setData(0, Qt::UserRole, QVariant::fromValue(platform.id));
    platformItem->setFlags(platformItem->flags() | Qt::ItemIsUserCheckable);
    
    bool allChecked = state.showShip && state.showName && state.showTrack && 
                      state.showSensors && state.showWeapons && state.showEvents;
    bool allUnchecked = !state.showShip && !state.showName && !state.showTrack && 
                        !state.showSensors && !state.showWeapons && !state.showEvents;
    if (allChecked) {
        platformItem->setCheckState(0, Qt::Checked);
    } else if (allUnchecked) {
        platformItem->setCheckState(0, Qt::Unchecked);
    } else {
        platformItem->setCheckState(0, Qt::PartiallyChecked);
    }

    QTreeWidgetItem *shipItem = new QTreeWidgetItem(platformItem);
    shipItem->setText(0, QString::fromLocal8Bit("Ship Icon"));
    shipItem->setFlags(shipItem->flags() | Qt::ItemIsUserCheckable);
    shipItem->setCheckState(0, state.showShip ? Qt::Checked : Qt::Unchecked);

    QTreeWidgetItem *nameItem = new QTreeWidgetItem(platformItem);
    nameItem->setText(0, QString::fromLocal8Bit("Name Label"));
    nameItem->setFlags(nameItem->flags() | Qt::ItemIsUserCheckable);
    nameItem->setCheckState(0, state.showName ? Qt::Checked : Qt::Unchecked);

    QTreeWidgetItem *trackItem = new QTreeWidgetItem(platformItem);
    trackItem->setText(0, QString::fromLocal8Bit("Track"));
    trackItem->setFlags(trackItem->flags() | Qt::ItemIsUserCheckable);
    trackItem->setCheckState(0, state.showTrack ? Qt::Checked : Qt::Unchecked);

    if (!platform.sensors.isEmpty()) {
        QTreeWidgetItem *sensorsItem = new QTreeWidgetItem(platformItem);
        sensorsItem->setText(0, QString::fromLocal8Bit("Sensors"));
        sensorsItem->setFlags(sensorsItem->flags() | Qt::ItemIsUserCheckable);
        sensorsItem->setCheckState(0, state.showSensors ? Qt::Checked : Qt::Unchecked);

        for (const SensorInfo &sensor : platform.sensors) {
            QTreeWidgetItem *sensorItem = new QTreeWidgetItem(sensorsItem);
            sensorItem->setText(0, QString("%1 (%2nm)").arg(sensor.type).arg(sensor.range));
            sensorItem->setFlags(sensorItem->flags() | Qt::ItemIsUserCheckable);
            sensorItem->setCheckState(0, Qt::Checked);
        }
    }

    if (!platform.weapons.isEmpty()) {
        QTreeWidgetItem *weaponsItem = new QTreeWidgetItem(platformItem);
        weaponsItem->setText(0, QString::fromLocal8Bit("Weapons"));
        weaponsItem->setFlags(weaponsItem->flags() | Qt::ItemIsUserCheckable);
        weaponsItem->setCheckState(0, state.showWeapons ? Qt::Checked : Qt::Unchecked);

        for (const WeaponInfo &weapon : platform.weapons) {
            QTreeWidgetItem *weaponItem = new QTreeWidgetItem(weaponsItem);
            weaponItem->setText(0, QString("%1 (%2nm)").arg(weapon.type).arg(weapon.range));
            weaponItem->setFlags(weaponItem->flags() | Qt::ItemIsUserCheckable);
            weaponItem->setCheckState(0, Qt::Checked);
        }
    }

    QTreeWidgetItem *eventsItem = new QTreeWidgetItem(platformItem);
    eventsItem->setText(0, QString::fromLocal8Bit("Events"));
    eventsItem->setFlags(eventsItem->flags() | Qt::ItemIsUserCheckable);
    eventsItem->setCheckState(0, state.showEvents ? Qt::Checked : Qt::Unchecked);
}

void PlatformControlPanel::onApplyClicked()
{
    DisplayStateMap stateMap;
    collectStates(stateMap);
    emit displayStateChanged(stateMap);
}

void PlatformControlPanel::onResetClicked()
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);
        campItem->setCheckState(0, Qt::Checked);

        for (int j = 0; j < campItem->childCount(); j++) {
            QTreeWidgetItem *platformItem = campItem->child(j);
            platformItem->setCheckState(0, Qt::Checked);

            for (int k = 0; k < platformItem->childCount(); k++) {
                QTreeWidgetItem *childItem = platformItem->child(k);
                childItem->setCheckState(0, Qt::Checked);

                for (int l = 0; l < childItem->childCount(); l++) {
                    QTreeWidgetItem *subChildItem = childItem->child(l);
                    subChildItem->setCheckState(0, Qt::Checked);
                }
            }
        }
    }
}

void PlatformControlPanel::onSearchTextChanged(const QString &text)
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);
        bool campVisible = false;

        for (int j = 0; j < campItem->childCount(); j++) {
            QTreeWidgetItem *platformItem = campItem->child(j);
            QString platformText = platformItem->text(0).toLower();
            bool platformVisible = text.isEmpty() || platformText.contains(text.toLower());
            platformItem->setHidden(!platformVisible);
            campVisible |= platformVisible;
        }

        campItem->setHidden(!campVisible && !text.isEmpty());
    }
}

void PlatformControlPanel::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    if (m_updatingCheckState) {
        return;
    }

    m_updatingCheckState = true;

    Qt::CheckState state = item->checkState(column);

    for (int i = 0; i < item->childCount(); i++) {
        item->child(i)->setCheckState(column, state);
    }

    if (item->parent()) {
        QTreeWidgetItem *parent = item->parent();
        bool allChecked = true;
        bool allUnchecked = true;

        for (int i = 0; i < parent->childCount(); i++) {
            if (parent->child(i)->checkState(column) == Qt::Checked) {
                allUnchecked = false;
            } else {
                allChecked = false;
            }
        }

        if (allChecked) {
            parent->setCheckState(column, Qt::Checked);
        } else if (allUnchecked) {
            parent->setCheckState(column, Qt::Unchecked);
        } else {
            parent->setCheckState(column, Qt::PartiallyChecked);
        }
    }

    m_updatingCheckState = false;
}

void PlatformControlPanel::collectStates(DisplayStateMap &stateMap)
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);

        for (int j = 0; j < campItem->childCount(); j++) {
            QTreeWidgetItem *platformItem = campItem->child(j);
            QString platformId = platformItem->data(0, Qt::UserRole).toString();

            PlatformDisplayState state;
            state.platformId = platformId;

            bool campUnchecked = (campItem->checkState(0) == Qt::Unchecked);
            bool platformUnchecked = (platformItem->checkState(0) == Qt::Unchecked);

            bool hasShip = false;
            bool hasName = false;
            bool hasTrack = false;
            bool hasSensors = false;
            bool hasWeapons = false;
            bool hasEvents = false;

            for (int k = 0; k < platformItem->childCount(); k++) {
                QTreeWidgetItem *childItem = platformItem->child(k);
                QString itemText = childItem->text(0);
                bool checked = childItem->checkState(0) == Qt::Checked;

                if (itemText.contains(QString::fromLocal8Bit("Ship Icon"))) {
                    hasShip = true;
                    state.showShip = !campUnchecked && !platformUnchecked && checked;
                } else if (itemText.contains(QString::fromLocal8Bit("Name Label"))) {
                    hasName = true;
                    state.showName = !campUnchecked && !platformUnchecked && checked;
                } else if (itemText.contains(QString::fromLocal8Bit("Track"))) {
                    hasTrack = true;
                    state.showTrack = !campUnchecked && !platformUnchecked && checked;
                } else if (itemText.contains(QString::fromLocal8Bit("Sensors"))) {
                    hasSensors = true;
                    state.showSensors = !campUnchecked && !platformUnchecked && checked;

                    for (int l = 0; l < childItem->childCount(); l++) {
                        QTreeWidgetItem *sensorItem = childItem->child(l);
                        ComponentState sensorState;
                        sensorState.componentId = sensorItem->text(0);
                        sensorState.componentName = sensorItem->text(0).split(" ").first();
                        sensorState.componentType = "sensor";
                        sensorState.enabled = checked && (sensorItem->checkState(0) == Qt::Checked);
                        state.sensors.append(sensorState);
                    }
                } else if (itemText.contains(QString::fromLocal8Bit("Weapons"))) {
                    hasWeapons = true;
                    state.showWeapons = !campUnchecked && !platformUnchecked && checked;

                    for (int l = 0; l < childItem->childCount(); l++) {
                        QTreeWidgetItem *weaponItem = childItem->child(l);
                        ComponentState weaponState;
                        weaponState.componentId = weaponItem->text(0);
                        weaponState.componentName = weaponItem->text(0).split(" ").first();
                        weaponState.componentType = "weapon";
                        weaponState.enabled = checked && (weaponItem->checkState(0) == Qt::Checked);
                        state.weapons.append(weaponState);
                    }
                } else if (itemText.contains(QString::fromLocal8Bit("Events"))) {
                    hasEvents = true;
                    state.showEvents = !campUnchecked && !platformUnchecked && checked;
                }
            }

            if (!hasShip) state.showShip = true;
            if (!hasName) state.showName = true;
            if (!hasTrack) state.showTrack = true;
            if (!hasSensors) state.showSensors = true;
            if (!hasWeapons) state.showWeapons = true;
            if (!hasEvents) state.showEvents = true;

            stateMap[platformId] = state;
        }
    }
}

QString PlatformControlPanel::campToString(CampType camp)
{
    switch (camp) {
    case Camp_Friendly: return QString::fromLocal8Bit("Friendly");
    case Camp_Enemy: return QString::fromLocal8Bit("Enemy");
    case Camp_Neutral: return QString::fromLocal8Bit("Neutral");
    default: return QString::fromLocal8Bit("Unknown");
    }
}