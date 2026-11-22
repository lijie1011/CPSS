/**
 * @file platformcontrolpanel.cpp
 * @brief 平台控制面板实现
 * @details 本类提供平台显示状态的可视化控制界面，允许用户勾选/取消勾选
 *          每个平台的显示选项。
 */

#include "platformcontrolpanel.h"

#include <QApplication>

/**
 * @brief 构造函数
 * @param parent 父窗口
 */
PlatformControlPanel::PlatformControlPanel(QWidget *parent)
    : QDialog(parent),
      m_applyScope(ApplyScope::Single),
      m_updatingCheckState(false)
{
    setWindowTitle(QStringLiteral("平台控制面板"));
    setMinimumSize(500, 600);
    initUI();
}

/**
 * @brief 析构函数
 */
PlatformControlPanel::~PlatformControlPanel()
{
}

/**
 * @brief 初始化用户界面
 */
void PlatformControlPanel::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部：作用范围 + 搜索
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    QLabel *scopeLabel = new QLabel(QStringLiteral("作用范围:"), this);
    topLayout->addWidget(scopeLabel);

    m_scopeCombo = new QComboBox(this);
    m_scopeCombo->addItem(QStringLiteral("所有"), (int)ApplyScope::All);
    m_scopeCombo->addItem(QStringLiteral("红方"), (int)ApplyScope::Red);
    m_scopeCombo->addItem(QStringLiteral("蓝方"), (int)ApplyScope::Blue);
    m_scopeCombo->addItem(QStringLiteral("单个平台"), (int)ApplyScope::Single);
    m_scopeCombo->setCurrentIndex(3);   // 默认"单个平台"
    connect(m_scopeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlatformControlPanel::onScopeChanged);
    topLayout->addWidget(m_scopeCombo);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("搜索平台..."));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PlatformControlPanel::onSearchTextChanged);
    topLayout->addWidget(m_searchEdit);

    mainLayout->addLayout(topLayout);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &PlatformControlPanel::onTreeItemClicked);
    connect(m_treeWidget, &QTreeWidget::currentItemChanged,
            this, &PlatformControlPanel::onTreeCurrentChanged);
    mainLayout->addWidget(m_treeWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_applyBtn = new QPushButton(QStringLiteral("应用"), this);
    connect(m_applyBtn, &QPushButton::clicked, this, &PlatformControlPanel::onApplyClicked);
    btnLayout->addWidget(m_applyBtn);

    m_resetBtn = new QPushButton(QStringLiteral("重置"), this);
    connect(m_resetBtn, &QPushButton::clicked, this, &PlatformControlPanel::onResetClicked);
    btnLayout->addWidget(m_resetBtn);

    m_closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    connect(m_closeBtn, &QPushButton::clicked, this, &PlatformControlPanel::onCloseClicked);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
}

/**
 * @brief 使用数据初始化面板
 * @param data 动态目标数据
 * @param currentStates 当前显示状态
 */
void PlatformControlPanel::initWithData(const DynamicObjects &data, const DisplayStateMap &currentStates)
{
    m_currentData = data;
    populateTree(data, currentStates);
}

/**
 * @brief 填充树控件
 * @param data 动态目标数据
 * @param currentStates 当前显示状态
 */
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
    campOrder << Camp_Mine << Camp_Friendly << Camp_Enemy << Camp_Neutral << Camp_Unknown;

    for (CampType camp : campOrder) {
        if (!groups.contains(camp)) continue;

        QString campName = campToString(camp);
        QTreeWidgetItem *campItem = new QTreeWidgetItem(m_treeWidget);
        campItem->setText(0, QString("%1 (%2)").arg(campName).arg(groups[camp].size()));
        campItem->setData(0, Qt::UserRole, (int)camp);   // 存储阵营类型，供批量范围筛选
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
                state.showTrack = false;
                state.showSensors = false;
                state.showWeapons = false;
                state.showEvents = true;
            }
            addPlatformNode(campItem, platform, state);
        }
    }

    m_treeWidget->expandAll();
}

/**
 * @brief 向树控件添加平台节点
 * @param campItem 阵营父节点
 * @param platform 平台数据
 * @param state 显示状态
 */
void PlatformControlPanel::addPlatformNode(QTreeWidgetItem *campItem, const PlatformData &platform, const PlatformDisplayState &state)
{
    QTreeWidgetItem *platformItem = new QTreeWidgetItem(campItem);
    platformItem->setText(0, QString("%1 (%2)").arg(platform.id).arg(platform.name));
    platformItem->setData(0, Qt::UserRole, QVariant::fromValue(platform.id));
    platformItem->setFlags(platformItem->flags() | Qt::ItemIsUserCheckable);
    
    bool hasChecked = state.showShip || state.showName || state.showTrack || 
                      state.showSensors || state.showWeapons || state.showEvents;
    platformItem->setCheckState(0, hasChecked ? Qt::Checked : Qt::Unchecked);

    QTreeWidgetItem *shipItem = new QTreeWidgetItem(platformItem);
    shipItem->setText(0, QStringLiteral("舰船图标"));
    shipItem->setFlags(shipItem->flags() | Qt::ItemIsUserCheckable);
    shipItem->setCheckState(0, state.showShip ? Qt::Checked : Qt::Unchecked);

    QTreeWidgetItem *nameItem = new QTreeWidgetItem(platformItem);
    nameItem->setText(0, QStringLiteral("名称标签"));
    nameItem->setFlags(nameItem->flags() | Qt::ItemIsUserCheckable);
    nameItem->setCheckState(0, state.showName ? Qt::Checked : Qt::Unchecked);

    QTreeWidgetItem *trackItem = new QTreeWidgetItem(platformItem);
    trackItem->setText(0, QStringLiteral("航迹"));
    trackItem->setFlags(trackItem->flags() | Qt::ItemIsUserCheckable);
    trackItem->setCheckState(0, state.showTrack ? Qt::Checked : Qt::Unchecked);

    if (!platform.sensors.isEmpty()) {
        QTreeWidgetItem *sensorsItem = new QTreeWidgetItem(platformItem);
        sensorsItem->setText(0, QStringLiteral("传感器"));
        sensorsItem->setFlags(sensorsItem->flags() | Qt::ItemIsUserCheckable);
        sensorsItem->setCheckState(0, state.showSensors ? Qt::Checked : Qt::Unchecked);

        for (const SensorInfo &sensor : platform.sensors) {
            QTreeWidgetItem *sensorItem = new QTreeWidgetItem(sensorsItem);
            sensorItem->setText(0, QString("%1 (%2nm)").arg(sensor.type).arg(sensor.range));
            sensorItem->setFlags(sensorItem->flags() | Qt::ItemIsUserCheckable);
            
            bool sensorEnabled = false;
            for (const ComponentState &cs : state.sensors) {
                if (cs.componentName == sensor.type) {
                    sensorEnabled = cs.enabled;
                    break;
                }
            }
            sensorItem->setCheckState(0, sensorEnabled ? Qt::Checked : Qt::Unchecked);
        }
    }

    if (!platform.weapons.isEmpty()) {
        QTreeWidgetItem *weaponsItem = new QTreeWidgetItem(platformItem);
        weaponsItem->setText(0, QStringLiteral("武器"));
        weaponsItem->setFlags(weaponsItem->flags() | Qt::ItemIsUserCheckable);
        weaponsItem->setCheckState(0, state.showWeapons ? Qt::Checked : Qt::Unchecked);

        for (const WeaponInfo &weapon : platform.weapons) {
            QTreeWidgetItem *weaponItem = new QTreeWidgetItem(weaponsItem);
            weaponItem->setText(0, QString("%1 (%2nm)").arg(weapon.type).arg(weapon.range));
            weaponItem->setFlags(weaponItem->flags() | Qt::ItemIsUserCheckable);
            
            bool weaponEnabled = false;
            for (const ComponentState &cs : state.weapons) {
                if (cs.componentName == weapon.type) {
                    weaponEnabled = cs.enabled;
                    break;
                }
            }
            weaponItem->setCheckState(0, weaponEnabled ? Qt::Checked : Qt::Unchecked);
        }
    }

    QTreeWidgetItem *eventsItem = new QTreeWidgetItem(platformItem);
    eventsItem->setText(0, QStringLiteral("事件"));
    eventsItem->setFlags(eventsItem->flags() | Qt::ItemIsUserCheckable);
    eventsItem->setCheckState(0, state.showEvents ? Qt::Checked : Qt::Unchecked);
}

/**
 * @brief 应用按钮点击处理函数
 */
void PlatformControlPanel::onApplyClicked()
{
    DisplayStateMap stateMap;
    collectStates(stateMap);
    emit displayStateChanged(stateMap);
}

/**
 * @brief 重置按钮点击处理函数
 */
void PlatformControlPanel::onResetClicked()
{
    m_updatingCheckState = true;

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);
        campItem->setCheckState(0, Qt::Checked);

        for (int j = 0; j < campItem->childCount(); j++) {
            QTreeWidgetItem *platformItem = campItem->child(j);
            platformItem->setCheckState(0, Qt::Checked);

            for (int k = 0; k < platformItem->childCount(); k++) {
                QTreeWidgetItem *childItem = platformItem->child(k);
                QString text = childItem->text(0);
                bool checked = (text == QStringLiteral("舰船图标") || 
                                text == QStringLiteral("名称标签"));
                childItem->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);

                for (int l = 0; l < childItem->childCount(); l++) {
                    QTreeWidgetItem *subChildItem = childItem->child(l);
                    subChildItem->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
                }
            }
        }
    }

    m_updatingCheckState = false;
    DisplayStateMap stateMap;
    collectStates(stateMap);
    emit displayStateChanged(stateMap);
}

/**
 * @brief 关闭按钮点击处理函数
 */
void PlatformControlPanel::onCloseClicked()
{
    DisplayStateMap stateMap;
    collectStates(stateMap);
    emit displayStateChanged(stateMap);
    close();
}

/**
 * @brief 搜索文本变化处理函数
 * @param text 搜索文本
 */
void PlatformControlPanel::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    applyFilter();
}

/**
 * @brief 作用范围下拉框切换处理
 */
void PlatformControlPanel::onScopeChanged(int index)
{
    m_applyScope = static_cast<ApplyScope>(m_scopeCombo->itemData(index).toInt());

    // 切到"单个平台"但尚未选中任何平台时，默认选第一个平台，避免空白列表
    if (m_applyScope == ApplyScope::Single && m_singlePlatformId.isEmpty()) {
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
            QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);
            if (campItem->childCount() > 0) {
                QTreeWidgetItem *first = campItem->child(0);
                m_singlePlatformId = first->data(0, Qt::UserRole).toString();
                break;
            }
        }
    }

    applyFilter();
}

/**
 * @brief 应用过滤（作用范围 + 搜索文本）
 * @details 范围决定显示哪些阵营/平台，搜索进一步按名称过滤，两者为 AND 关系。
 *          - All  : 所有平台
 *          - Red  : 仅 Camp_Mine（红方）阵营
 *          - Blue : 仅 Camp_Enemy（蓝方）阵营
 *          - Single: 仅 m_singlePlatformId 指定的单个平台（未选中则不显示任何平台）
 */
void PlatformControlPanel::applyFilter()
{
    if (!m_treeWidget) return;
    const QString keyword = m_searchEdit->text().trimmed().toLower();

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);
        CampType camp = (CampType)campItem->data(0, Qt::UserRole).toInt();

        // 1) 阵营级范围过滤
        bool campInScope = false;
        switch (m_applyScope) {
        case ApplyScope::All:    campInScope = true; break;
        case ApplyScope::Red:    campInScope = (camp == Camp_Mine); break;
        case ApplyScope::Blue:   campInScope = (camp == Camp_Enemy); break;
        case ApplyScope::Single: campInScope = true; break;  // 单个平台在下面按 id 过滤
        }

        bool campHasVisible = false;
        for (int j = 0; j < campItem->childCount(); j++) {
            QTreeWidgetItem *platformItem = campItem->child(j);
            const QString pid = platformItem->data(0, Qt::UserRole).toString();
            const QString ptext = platformItem->text(0).toLower();

            // 2) 平台级：Single 按选中 id（有搜索词时放行，便于切换目标）；其他按阵营范围
            bool platformInScope = false;
            if (m_applyScope == ApplyScope::Single) {
                platformInScope = !keyword.isEmpty() || (!m_singlePlatformId.isEmpty() && pid == m_singlePlatformId);
            } else {
                platformInScope = campInScope;
            }

            // 3) 搜索文本过滤（AND）
            bool matchSearch = keyword.isEmpty() || ptext.contains(keyword);
            bool visible = platformInScope && matchSearch;

            platformItem->setHidden(!visible);
            campHasVisible |= visible;
        }

        campItem->setHidden(!campHasVisible);
    }
}

/**
 * @brief 树当前项变化：记录"单个平台"范围下的选中平台
 */
void PlatformControlPanel::onTreeCurrentChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    QTreeWidgetItem *platform = platformItemOf(current);
    if (platform) {
        m_singlePlatformId = platform->data(0, Qt::UserRole).toString();
    }
    if (m_applyScope == ApplyScope::Single) {
        applyFilter();
    }
}

/**
 * @brief 判断是否为设置项（父节点是平台节点，即父节点携带 platformId）
 */
bool PlatformControlPanel::isSettingItem(QTreeWidgetItem *item) const
{
    if (!item || !item->parent()) return false;
    QTreeWidgetItem *parent = item->parent();
    // 平台节点的父节点是阵营节点（顶层）；设置项的父节点就是平台节点
    return parent->parent() != nullptr && !parent->data(0, Qt::UserRole).toString().isEmpty();
}

/**
 * @brief 获取设置项标识键
 */
QString PlatformControlPanel::settingKey(QTreeWidgetItem *item) const
{
    if (!item) return QString();
    const QString text = item->text(0);
    const QStringList keys = {
        QStringLiteral("舰船图标"), QStringLiteral("名称标签"), QStringLiteral("航迹"),
        QStringLiteral("传感器"), QStringLiteral("武器"), QStringLiteral("事件")
    };
    for (const QString &k : keys) {
        if (text.contains(k)) return k;
    }
    return QString();
}

/**
 * @brief 向上找到携带 platformId 的平台节点
 */
QTreeWidgetItem* PlatformControlPanel::platformItemOf(QTreeWidgetItem *item) const
{
    while (item) {
        if (!item->data(0, Qt::UserRole).toString().isEmpty()) return item;
        item = item->parent();
    }
    return nullptr;
}

/**
 * @brief 根据作用范围获取目标平台节点列表
 */
QList<QTreeWidgetItem*> PlatformControlPanel::platformsInScope(ApplyScope scope, QTreeWidgetItem *clickedPlatform) const
{
    QList<QTreeWidgetItem*> result;
    if (scope == ApplyScope::Single) {
        if (clickedPlatform) result.append(clickedPlatform);
        return result;
    }

    // All / Red / Blue：遍历所有顶层阵营节点
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *campItem = m_treeWidget->topLevelItem(i);
        CampType camp = (CampType)campItem->data(0, Qt::UserRole).toInt();

        bool campMatch = false;
        if (scope == ApplyScope::All) campMatch = true;
        else if (scope == ApplyScope::Red) campMatch = (camp == Camp_Mine);
        else if (scope == ApplyScope::Blue) campMatch = (camp == Camp_Enemy);
        if (!campMatch) continue;

        for (int j = 0; j < campItem->childCount(); j++) {
            result.append(campItem->child(j));
        }
    }
    return result;
}

/**
 * @brief 在平台节点下查找指定设置项
 */
QTreeWidgetItem* PlatformControlPanel::findSettingItem(QTreeWidgetItem *platformItem, const QString &key) const
{
    if (!platformItem) return nullptr;
    for (int i = 0; i < platformItem->childCount(); i++) {
        QTreeWidgetItem *child = platformItem->child(i);
        if (child->text(0).contains(key)) return child;
    }
    return nullptr;
}

/**
 * @brief 树控件项点击处理，联动更新父子节点勾选状态
 * @param item 被点击的树控件项
 * @param column 列索引
 */
void PlatformControlPanel::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    if (m_updatingCheckState) {
        return;
    }

    m_updatingCheckState = true;

    Qt::CheckState state = item->checkState(column);

    // ===== 批量设置：作用范围不是单个平台时 =====
    // 情况A：点击"设置项" → 把该设置批量应用到范围内所有平台
    // 情况B：点击"平台节点" → 把该平台的整组显示状态批量应用到范围内所有平台
    if (m_applyScope != ApplyScope::Single) {
        QTreeWidgetItem *clickedPlatform = platformItemOf(item);
        const bool clickSetting = isSettingItem(item);
        const bool clickPlatform = clickedPlatform && (item == clickedPlatform);

        if (clickSetting || clickPlatform) {
            const QString key = clickSetting ? settingKey(item) : QString();
            QList<QTreeWidgetItem*> targets = platformsInScope(m_applyScope, clickedPlatform);

            for (QTreeWidgetItem *target : targets) {
                if (clickSetting) {
                    // 只应用单个设置项
                    QTreeWidgetItem *setting = findSettingItem(target, key);
                    if (!setting) continue;
                    setting->setCheckState(column, state);
                    // 传感器/武器：同步其下所有设备子项
                    if (key == QStringLiteral("传感器") || key == QStringLiteral("武器")) {
                        for (int i = 0; i < setting->childCount(); i++) {
                            setting->child(i)->setCheckState(column, state);
                        }
                    }
                } else {
                    // 应用整组：平台下所有设置项及其设备子项
                    for (int i = 0; i < target->childCount(); i++) {
                        QTreeWidgetItem *setting = target->child(i);
                        setting->setCheckState(column, state);
                        for (int j = 0; j < setting->childCount(); j++) {
                            setting->child(j)->setCheckState(column, state);
                        }
                    }
                }

                // 重算该平台节点勾选状态（任意子项勾选即平台勾选）
                bool anyChecked = false;
                for (int i = 0; i < target->childCount(); i++) {
                    if (target->child(i)->checkState(column) != Qt::Unchecked) {
                        anyChecked = true;
                        break;
                    }
                }
                target->setCheckState(column, anyChecked ? Qt::Checked : Qt::Unchecked);
            }

            m_updatingCheckState = false;
            return;
        }
    }

    // ===== 原有逻辑：子节点跟随 + 父节点反推 =====
    for (int i = 0; i < item->childCount(); i++) {
        item->child(i)->setCheckState(column, state);
    }

    if (item->parent()) {
        QTreeWidgetItem *parent = item->parent();
        bool hasChecked = false;
        bool allChecked = true;

        for (int i = 0; i < parent->childCount(); i++) {
            Qt::CheckState childState = parent->child(i)->checkState(column);
            if (childState == Qt::Checked || childState == Qt::PartiallyChecked) {
                hasChecked = true;
            }
            if (childState != Qt::Checked) {
                allChecked = false;
            }
        }

        if (hasChecked) {
            parent->setCheckState(column, Qt::Checked);
        } else {
            parent->setCheckState(column, Qt::Unchecked);
        }

        if (parent->parent()) {
            QTreeWidgetItem *grandParent = parent->parent();
            bool grandHasChecked = false;
            for (int i = 0; i < grandParent->childCount(); i++) {
                Qt::CheckState childState = grandParent->child(i)->checkState(column);
                if (childState == Qt::Checked || childState == Qt::PartiallyChecked) {
                    grandHasChecked = true;
                    break;
                }
            }
            grandParent->setCheckState(column, grandHasChecked ? Qt::Checked : Qt::Unchecked);
        }
    }

    m_updatingCheckState = false;
}

/**
 * @brief 从树控件收集所有平台的显示状态
 * @param stateMap 输出的显示状态映射表
 */
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

                if (itemText.contains(QStringLiteral("舰船图标"))) {
                    hasShip = true;
                    state.showShip = !campUnchecked && !platformUnchecked && checked;
                } else if (itemText.contains(QStringLiteral("名称标签"))) {
                    hasName = true;
                    state.showName = !campUnchecked && !platformUnchecked && checked;
                } else if (itemText.contains(QStringLiteral("航迹"))) {
                    hasTrack = true;
                    state.showTrack = !campUnchecked && !platformUnchecked && checked;
                } else if (itemText.contains(QStringLiteral("传感器"))) {
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
                } else if (itemText.contains(QStringLiteral("武器"))) {
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
                } else if (itemText.contains(QStringLiteral("事件"))) {
                    hasEvents = true;
                    state.showEvents = !campUnchecked && !platformUnchecked && checked;
                }
            }

            if (!hasShip) state.showShip = true;
            if (!hasName) state.showName = true;
            if (!hasTrack) state.showTrack = false;
            if (!hasSensors) state.showSensors = false;
            if (!hasWeapons) state.showWeapons = false;
            if (!hasEvents) state.showEvents = true;

            stateMap[platformId] = state;
        }
    }
}

/**
 * @brief 阵营类型转字符串
 * @param camp 阵营类型
 * @return 阵营名称字符串
 */
QString PlatformControlPanel::campToString(CampType camp)
{
    // 与 AFSIM side 及批量范围选项保持一致：红方 / 蓝方
    switch (camp) {
    case Camp_Mine: return QStringLiteral("红方");
    case Camp_Enemy: return QStringLiteral("蓝方");
    case Camp_Friendly: return QStringLiteral("友方");
    case Camp_Neutral: return QStringLiteral("中立");
    case Camp_Unknown: return QStringLiteral("未知");
    default: return QStringLiteral("未知");
    }
}
