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
    QLabel *label;                   ///< 显示标签
    QGraphicsLineItem *connectingLine; ///< 连接线
    bool isDragging;                 ///< 当前是否正在拖拽
    QPoint dragOffset;               ///< 拖拽偏移量

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
     * @brief 更新事件标记
     */
    void updateEventMarkers();

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
     * @brief 通过标签查找属性框
     * @param label 标签
     * @return 属性框指针
     */
    PropertyBox* findPropertyBoxByLabel(QLabel *label);
    
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
    QMap<QString, QList<QGraphicsItem*>> m_eventMarkerItems; ///< 事件标记图元映射

    QList<PropertyBox> m_propertyBoxes;         ///< 属性框列表
    QLabel *m_overviewLabel;                    ///< 鹰眼图标签
    QRect m_overviewViewport;                   ///< 鹰眼图视口
};

#endif
