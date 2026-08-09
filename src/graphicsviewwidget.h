/**
 * @file graphicsviewwidget.h
 * @brief 海图视图部件类定义
 * @details 本类是态势海图的核心显示部件，继承自 QGraphicsView，
 *          负责渲染海图、平台、传感器/武器作用范围、航迹、事件标记等
 *          图形元素。
 */

#ifndef GRAPHICSVIEWWIDGET_H
#define GRAPHICSVIEWWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QString>
#include <QSet>
#include <QLabel>
#include <QRect>
#include <QMap>

#include "dynamicdata.h"
#include "displaystate.h"
#include "platformitem.h"
#include "propertyboxitem.h"

/**
 * @struct PropertyBox
 * @brief 属性信息框结构体
 * @details 用于显示平台属性信息的浮动框
 */
struct PropertyBox {
    QString id;                      ///< 平台 ID
    QString name;                    ///< 平台名称
    bool isOwnShip;                  ///< 是否为本舰
    double lon;                      ///< 经度
    double lat;                      ///< 纬度
    double heading;                  ///< 航向
    double speed;                    ///< 速度
    PropertyBoxItem *item;           ///< 场景内属性框图元（附着/脱离与偏移由其自身持有）
    QGraphicsLineItem *connectingLine; ///< 连接线

    /**
     * @brief 相等运算符重载
     * @param other 另一个 PropertyBox
     * @return 相等返回 true
     */
    bool operator==(const PropertyBox &other) const {
        return id == other.id && isOwnShip == other.isOwnShip;
    }
};

/**
 * @class GraphicsViewWidget
 * @brief 海图视图部件类
 * @details 继承自 QGraphicsView，负责态势海图的渲染与交互
 */
class GraphicsViewWidget : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父部件
     */
    explicit GraphicsViewWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~GraphicsViewWidget();

    /**
     * @brief 更新动态数据
     * @param data 动态目标数据
     */
    void updateDynamicData(const DynamicObjects &data);
    
    /**
     * @brief 单平台增量更新（新增或更新）
     * @param platform 单个平台数据
     * @details 连接 DataManager::platformUpdated，仅更新/新建该 id 对应的图元，
     *          实现多目标实时显示，避免每次全量重建。内部同步维护 m_dynamicData
     *          以保证鼠标命中测试与控制面板等仍可用。
     */
    void updateSinglePlatform(const PlatformData &platform);
    
    /**
     * @brief 单平台增量删除
     * @param id 被移除（删除或过期）平台的 ID
     * @details 连接 DataManager::platformRemoved，删除该 id 对应的图元及其
     *          传感器/武器范围、航迹、属性框等附属元素，避免残留。
     */
    void removeSinglePlatform(const QString &id);
    
    /**
     * @brief 获取当前动态数据
     * @return 动态目标数据引用
     */
    const DynamicObjects& getDynamicData() const { return m_dynamicData; }
    
    /**
     * @brief 获取显示状态映射
     * @return 显示状态映射引用
     */
    const DisplayStateMap& getDisplayStates() const { return m_displayStates; }
    
    /**
     * @brief 设置 Enclib 就绪状态
     * @param ready 是否就绪
     */
    void setEnclibReady(bool ready);
    
    /**
     * @brief 检查 Enclib 是否已就绪
     * @return 就绪返回 true
     */
    bool isEnclibReady() const { return m_enclibReady; }

signals:
    /**
     * @brief 地理坐标位置更新信号
     * @param pos 屏幕坐标
     */
    void updateGeoPosition(QPoint pos);

public slots:
    /**
     * @brief 更新海图
     */
    void updateChart();
    
    /**
     * @brief 放大视图
     */
    void zoomIn();
    
    /**
     * @brief 缩小视图
     */
    void zoomOut();
    
    /**
     * @brief 设置海图中心
     * @param lon 经度
     * @param lat 纬度
     */
    void setChartCenter(double lon, double lat);
    
    /**
     * @brief 更新显示状态
     * @param stateMap 显示状态映射
     */
    void updateDisplayState(const DisplayStateMap &stateMap);

    /**
     * @brief 事件新增时刷新海图事件标记
     * @param event 新增的事件（此处仅作触发信号使用，实际按全量事件重绘标记）
     * @details 连接 DataManager::eventAdded，替代原先依赖全量 dynamicDataChanged
     *          的兜底刷新。内部调用 updateEventMarkers()，使新增事件的图标
     *          立即出现在对应平台旁。
     */
    void refreshEventMarkers(const SpecialEvent &event);

private slots:
    /**
     * @brief 属性框被拖动的响应槽
     * @param item 被拖动的属性框图元
     * @details item 内部已置为脱离/固定状态，此处仅刷新其连接线。
     */
    void onPropertyBoxMoved(PropertyBoxItem *item);

    /**
     * @brief 属性框被双击的响应槽
     * @param item 被双击的属性框图元
     * @details 删除该属性框及其连接线（与原双击删除交互一致）。
     */
    void onPropertyBoxDoubleClicked(PropertyBoxItem *item);

protected:
    /**
     * @brief 重绘事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event) override;
    
    /**
     * @brief 鼠标按下事件
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    
    /**
     * @brief 鼠标移动事件
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    
    /**
     * @brief 鼠标释放事件
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *event) override;
    
    /**
     * @brief 滚轮事件
     * @param event 滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;
    
    /**
     * @brief 尺寸变化事件
     * @param event 尺寸事件
     */
    void resizeEvent(QResizeEvent *event) override;
    
    /**
     * @brief 事件过滤器
     * @param obj 目标对象
     * @param event 事件
     * @return 成功处理返回 true
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    /**
     * @brief 更新地图背景
     */
    void updateMapBackground();
    
    /**
     * @brief 更新平台图元
     */
    void updatePlatformItems();
    
    /**
     * @brief 更新传感器/武器作用范围
     */
    void updateSensorWeaponRanges();
    
    /**
     * @brief 更新航迹
     */
    void updateTracks();
    
    /**
     * @brief 更新事件标记（统一处理两种呈现方式）
     * @details 两种方式共用同一套通信与容器 m_eventMarkerItems：
     *          - 方式1（跟随平台）：事件带 targetId，标记画在对应平台当前位置，
     *            方形字母图标，随平台移动、随平台删除而清理，键为平台ID；
     *          - 方式2（地理标绘）：事件无 targetId，按事件自身 lon/lat 标绘，
     *            菱形标志 + 事件名文字，位置固定、不跟随平台，键为事件ID。
     */
    void updateEventMarkers();

    /**
     * @brief 更新单个平台上方的设备状态徽标条（横向一排）
     * @param platform 平台数据（提供阵营/事件关联等）
     * @param x 平台图标在场景中的 X 坐标
     * @param y 平台图标在场景中的 Y 坐标
     * @details 在平台图标正上方横向排布若干小徽标，状态数据复用 m_displayStates：
     *          - 传感器开启：displayState.sensors 任一 enabled 为真时显示“S”（青色）；
     *          - 武器就绪：displayState.weapons 任一 enabled 为真时显示“W”（橙色）；
     *          - 通信/受损：由该平台最新事件类型派生（Event_Lost→通信中断，
     *            Event_Damage→受损），无对应事件则不显示。
     *          徽标按平台ID存于 m_statusStripItems，每拍整条重建（1Hz 开销极小），
     *          随平台移动而跟随、随平台删除而清理。
     */
    void updatePlatformStatusStrip(const PlatformData &platform, int x, int y);

    /**
     * @brief 地理坐标转屏幕坐标
     * @param lon 经度
     * @param lat 纬度
     * @param x 输出屏幕 X 坐标
     * @param y 输出屏幕 Y 坐标
     * @return 转换成功返回 true
     */
    bool geoToScreen(double lon, double lat, int &x, int &y);
    
    /**
     * @brief 更新连接线
     * @param box 属性框
     */
    void updateConnectingLine(PropertyBox &box);
    
    /**
     * @brief 更新所有连接线
     */
    void updateAllConnectingLines();
    
    /**
     * @brief 通过图元查找属性框
     * @param item 属性框图元
     * @return 属性框指针
     */
    PropertyBox* findPropertyBoxByItem(PropertyBoxItem *item);
    
    /**
     * @brief 通过 ID 查找属性框
     * @param id 平台 ID
     * @param isOwnShip 是否为本舰
     * @return 属性框指针
     */
    PropertyBox* findPropertyBoxById(const QString &id, bool isOwnShip);
    
    /**
     * @brief 检查点是否在平台内
     * @param x 屏幕 X 坐标
     * @param y 屏幕 Y 坐标
     * @param item 平台图元
     * @return 在平台内返回 true
     */
    bool isPointInPlatform(int x, int y, PlatformItem *item);
    
    /**
     * @brief 检查点是否在舰船图标内
     * @param x 屏幕 X 坐标
     * @param y 屏幕 Y 坐标
     * @param shipX 舰船 X 坐标
     * @param shipY 舰船 Y 坐标
     * @return 在舰船内返回 true
     */
    bool isPointInShip(int x, int y, int shipX, int shipY);
    
    /**
     * @brief 检查点是否在事件标记内
     * @param x 屏幕 X 坐标
     * @param y 屏幕 Y 坐标
     * @param event 事件数据
     * @return 在事件内返回 true
     */
    bool isPointInEvent(int x, int y, const SpecialEvent &event);
    
    /**
     * @brief 创建属性框
     * @param platform 平台数据
     */
    void createPropertyBox(const PlatformData &platform);
    
    /**
     * @brief 创建事件信息框
     * @param event 事件数据
     */
    void createEventInfoBox(const SpecialEvent &event);
    
    /**
     * @brief 销毁属性框
     * @param box 属性框
     */
    void destroyPropertyBox(PropertyBox *box);
    
    /**
     * @brief 初始化鹰眼图
     */
    void initOverviewMap();
    
    /**
     * @brief 更新鹰眼图
     */
    void updateOverviewMap();
    
    /**
     * @brief 绘制鹰眼图内容
     */
    void drawOverviewMapContent();
    
    /**
     * @brief 绘制连接线
     */
    void drawConnectingLines();

    QPoint m_leftMousePressPt;                  ///< 鼠标左键按下位置
    QPoint m_lastLeftMousePt;                   ///< 上一次鼠标位置
    QImage m_storedViewImg;                     ///< 存储的视图图像
    QImage m_overviewImage;                     ///< 鹰眼图图像
    double m_rotAngle;                          ///< 旋转角度
    bool m_drawOthers;                          ///< 是否绘制其他元素
    bool m_enclibReady;                         ///< Enclib 是否就绪
    DynamicObjects m_dynamicData;               ///< 动态数据
    DisplayStateMap m_displayStates;            ///< 显示状态映射

    QGraphicsScene *m_scene;                    ///< 图形场景
    QMap<QString, PlatformItem*> m_platformItems;           ///< 平台图元映射
    QMap<QString, QList<QGraphicsItem*>> m_sensorRangeItems; ///< 传感器作用范围图元映射
    QMap<QString, QList<QGraphicsItem*>> m_weaponRangeItems; ///< 武器作用范围图元映射
    QMap<QString, QGraphicsItem*> m_trackItems;             ///< 航迹图元映射
    QMap<QString, QList<QGraphicsItem*>> m_eventMarkerItems; ///< 事件标记图元映射（方式1按平台ID、方式2按事件ID共用同一容器）
    QMap<QString, QList<QGraphicsItem*>> m_statusStripItems; ///< 设备状态徽标条图元映射（按平台ID，横排显示传感器/武器/通信受损状态）

    QList<PropertyBox> m_propertyBoxes;         ///< 属性框列表
    QLabel *m_overviewLabel;                    ///< 鹰眼图标签
    QRect m_overviewViewport;                   ///< 鹰眼图视口
};

#endif
