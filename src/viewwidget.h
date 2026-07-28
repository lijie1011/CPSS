/**
 * @file viewwidget.h
 * @brief 视图部件类定义
 * @details 该类是态势图显示的核心部件，负责绘制海图背景、平台、事件标记、传感器/武器范围等元素，
 *          支持鼠标交互和缩放操作。
 * @date 2026-07-28
 */

#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QImage>
#include <QString>
#include <QSet>
#include <QLabel>
#include <QRect>

#include "dynamicdata.h"
#include "displaystate.h"

/**
 * @struct PropertyBox
 * @brief 属性信息框结构
 * @details 用于显示平台属性信息的浮动框
 */
struct PropertyBox {
    QString id;       ///< 平台ID
    QString name;     ///< 平台名称
    bool isOwnShip;   ///< 是否为本船
    double lon;       ///< 经度
    double lat;       ///< 纬度
    double heading;   ///< 航向
    double speed;     ///< 速度
    QLabel *label;    ///< 显示标签
    bool isDragging;  ///< 是否正在拖动
    QPoint dragOffset; ///< 拖动偏移量

    /**
     * @brief 相等运算符重载
     * @param other 另一个PropertyBox
     * @return 相等返回true
     */
    bool operator==(const PropertyBox &other) const {
        return id == other.id && isOwnShip == other.isOwnShip;
    }
};

/**
 * @class ViewWidget
 * @brief 视图部件类
 * @details 继承自QWidget，负责态势图的渲染和交互
 */
class ViewWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口部件
     */
    explicit ViewWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ViewWidget();

    /**
     * @brief 更新动态数据
     * @param data 动态对象数据
     */
    void updateDynamicData(const DynamicObjects &data);
    
    /**
     * @brief 获取当前动态数据
     * @return 动态对象数据引用
     */
    const DynamicObjects& getDynamicData() const { return m_dynamicData; }
    
    /**
     * @brief 获取显示状态映射
     * @return 显示状态映射引用
     */
    const DisplayStateMap& getDisplayStates() const { return m_displayStates; }
    
    /**
     * @brief 设置Enclib就绪状态
     * @param ready 是否就绪
     */
    void setEnclibReady(bool ready);
    
    /**
     * @brief 检查Enclib是否就绪
     * @return 就绪返回true
     */
    bool isEnclibReady() const { return m_enclibReady; }

signals:
    /**
     * @brief 更新地理坐标位置信号
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
     * @brief 重写绘制事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event) override;
    
    /**
     * @brief 重写鼠标按下事件
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    
    /**
     * @brief 重写鼠标移动事件
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    
    /**
     * @brief 重写鼠标释放事件
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *event) override;
    
    /**
     * @brief 重写滚轮事件
     * @param event 滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;
    
    /**
     * @brief 重写窗口大小改变事件
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent *event) override;
    
    /**
     * @brief 重写事件过滤器
     * @param obj 目标对象
     * @param event 事件
     * @return 处理成功返回true
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    /**
     * @brief 绘制平台
     * @param painter 绘制器
     * @param platform 平台数据
     */
    void drawPlatform(QPainter &painter, const PlatformData &platform);
    
    /**
     * @brief 绘制事件标记
     * @param painter 绘制器
     * @param x 屏幕X坐标
     * @param y 屏幕Y坐标
     * @param eventType 事件类型
     */
    void drawEventMarker(QPainter &painter, int x, int y, SpecialEventType eventType);
    
    /**
     * @brief 绘制传感器范围
     * @param painter 绘制器
     * @param x 屏幕X坐标
     * @param y 屏幕Y坐标
     * @param platform 平台数据
     */
    void drawSensorRanges(QPainter &painter, int x, int y, const PlatformData &platform);
    
    /**
     * @brief 绘制武器范围
     * @param painter 绘制器
     * @param x 屏幕X坐标
     * @param y 屏幕Y坐标
     * @param platform 平台数据
     */
    void drawWeaponRanges(QPainter &painter, int x, int y, const PlatformData &platform);
    
    /**
     * @brief 绘制独立事件
     * @param painter 绘制器
     * @param event 事件数据
     */
    void drawStandaloneEvent(QPainter &painter, const SpecialEvent &event);
    
    /**
     * @brief 绘制连接线
     * @param painter 绘制器
     */
    void drawConnectingLines(QPainter &painter);
    
    /**
     * @brief 地理坐标转屏幕坐标
     * @param lon 经度
     * @param lat 纬度
     * @param x 输出屏幕X坐标
     * @param y 输出屏幕Y坐标
     * @return 转换成功返回true
     */
    bool geoToScreen(double lon, double lat, int &x, int &y);
    
    /**
     * @brief 根据标签查找属性框
     * @param label 标签
     * @return 属性框指针
     */
    PropertyBox* findPropertyBoxByLabel(QLabel *label);
    
    /**
     * @brief 根据ID查找属性框
     * @param id 平台ID
     * @param isOwnShip 是否为本船
     * @return 属性框指针
     */
    PropertyBox* findPropertyBoxById(const QString &id, bool isOwnShip);
    
    /**
     * @brief 检查点是否在船只图标内
     * @param x 屏幕X坐标
     * @param y 屏幕Y坐标
     * @param shipX 船只X坐标
     * @param shipY 船只Y坐标
     * @return 在船只内返回true
     */
    bool isPointInShip(int x, int y, int shipX, int shipY);
    
    /**
     * @brief 检查点是否在事件标记内
     * @param x 屏幕X坐标
     * @param y 屏幕Y坐标
     * @param event 事件数据
     * @return 在事件内返回true
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
     * @brief 初始化概览图
     */
    void initOverviewMap();
    
    /**
     * @brief 更新概览图
     */
    void updateOverviewMap();
    
    /**
     * @brief 绘制概览图内容
     */
    void drawOverviewMapContent();
    
    /**
     * @brief 加载图标
     */
    void loadIcons();

    QPoint m_leftMousePressPt;  ///< 鼠标左键按下位置
    QPoint m_lastLeftMousePt;   ///< 上一次鼠标位置
    QImage m_storedViewImg;     ///< 存储的视图图像
    QImage m_overviewImage;     ///< 概览图图像
    double m_rotAngle;          ///< 旋转角度
    bool   m_drawOthers;        ///< 是否绘制其他元素
    bool   m_enclibReady;       ///< Enclib是否就绪
    DynamicObjects m_dynamicData;    ///< 动态数据
    DisplayStateMap m_displayStates; ///< 显示状态映射

    QList<PropertyBox> m_propertyBoxes;  ///< 属性框列表
    QLabel *m_overviewLabel;             ///< 概览图标签
    QRect m_overviewViewport;            ///< 概览图视口

    QImage m_redBoatIcon;      ///< 红方船只图标
    QImage m_redPlaneIcon;     ///< 红方飞机图标
    QImage m_purpleBoatIcon;   ///< 紫方船只图标
    QImage m_purplePlaneIcon;  ///< 紫方飞机图标
};

#endif