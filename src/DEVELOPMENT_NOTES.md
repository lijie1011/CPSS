# CPSS 项目开发笔记

## 一、问题修复记录

### 1.1 Reset 按钮失效问题

**问题描述**：点击 Reset 按钮没有复位操作

**根因**：`ViewWidget::setChartCenter()` 函数为空实现

**修复方案**：
- 在 `viewwidget.cpp` 中实现 `setChartCenter()` 函数
- 调用 `EnclViewCenter()` 和 `EnclViewSetScale()` 重置地图视图

**修改文件**：`src/viewwidget.cpp`

---

### 1.2 测试数据未触发

**问题描述**：测试数据没有自动生成

**根因**：`DataManager::startTestDataTimer()` 未被调用

**修复方案**：
- 在 `MainWindow::init()` 中调用 `startTestDataTimer(1000)`
- 连接 `DataManager::dynamicDataChanged` 信号到 `ViewWidget::updateDynamicData`

**修改文件**：`src/mainwindow.cpp`

---

### 1.3 目标在海图中不显示

**问题描述**：测试数据已传输，但目标未显示在海图上

**根因**：地图中心坐标 (121.0, 31.0) 与测试数据位置 (121.5, 31.2) 不匹配

**修复方案**：
- 更新 `EnclViewCenter()` 参数为 (121.5, 31.2)

**修改文件**：`src/mainwindow.cpp`

---

### 1.4 DLL 边界问题

**问题描述**：Logger 静态变量在 exe 和 dll 之间不共享

**修复方案**：
- 创建 `cpss_common` 库，使用 `CPSS_COMMON_API` 导出/导入
- Logger 移至 common 库，确保日志在整个应用中共享

**修改文件**：
- `src/common/logger.h`
- `src/common/logger.cpp`
- `src/CMakeLists.txt`

---

### 1.5 测试数据解析失败

**问题描述**：`initTestData()` 生成零平台

**根因**：JSON 对象中 `"type"` 字段被设置两次（先设为 `"platform"`，后被覆盖为 `"warship"`）

**修复方案**：
- 将船型字段改名为 `platformType`，保留 `"type": "platform"` 用于协议解析

**修改文件**：`src/datalib/datamanager.cpp`

---

## 二、功能实现

### 2.1 属性框功能

**功能列表**：
- 点击目标显示属性框
- 拖动属性框改变位置
- 双击属性框关闭
- 点击空白区域隐藏所有属性框
- 目标与属性框之间的虚线连接

**实现方式**：
- 使用 `QLabel` 作为属性框容器
- 通过 `eventFilter` 监听属性框鼠标事件
- 在 `paintEvent` 中绘制虚线连接

**修改文件**：
- `src/viewwidget.h`
- `src/viewwidget.cpp`

---

### 2.2 阵营和特殊事件

**阵营枚举**：
```cpp
enum CampType {
    Camp_Unknown,
    Camp_Friendly,   // 绿色
    Camp_Enemy,      // 红色
    Camp_Neutral     // 黄色
};
```

**事件类型**：
```cpp
enum SpecialEventType {
    Event_Unknown,
    Event_Attack,
    Event_Defense,
    Event_Alert,
    Event_MissionStart,
    Event_MissionEnd,
    Event_Contact,
    Event_Lost,
    Event_Damage,
    Event_Repair,
    Event_Custom
};
```

**事件数据结构**：
```cpp
struct SpecialEvent {
    QString eventId;
    SpecialEventType eventType;
    QString eventName;
    QString description;
    qint64 timestamp;
    QString targetId;
    QString sourceId;
    QJsonObject extraData;
};
```

**修改文件**：
- `src/datalib/dynamicdata.h`
- `src/datalib/datamanager.h`
- `src/datalib/datamanager.cpp`

---

### 2.3 事件图例说明

**功能**：在菜单栏添加 Help -> Event Legend 选项，显示图例说明

**快捷键**：F1

**修改文件**：
- `src/mainwindow.h`
- `src/mainwindow.cpp`

---

### 2.4 布局调整

**布局方案**：
- 工具栏在左侧，垂直排列
- 地图占据右侧大部分空间
- 水平布局，工具栏与地图并排

**修改文件**：
- `src/mainwindow.h`
- `src/mainwindow.cpp`

---

## 三、数据类接口架构设计

### 3.1 两种方案对比

#### 方案 A：继承模式（基类统一管理）

```
BaseDataManager (基类)
    ├── m_cache (每个实例独立)
    ├── m_lock
    ├── m_expireTimer
    └── internalAddData()

    ├── UdpDataManager (子类)
    ├── TcpDataManager (子类)
    └── HttpDataManager (子类)
```

**优点**：
- 代码复用性好
- 接口统一
- 实现简单

**缺点**：
- 每个子类有独立缓存，数据可能不一致
- 内存浪费（重复存储）
- 查询效率低（需要遍历所有子类）
- 高耦合

---

#### 方案 B：组合模式（共享缓存）

```
DataCache (独立类，全局唯一)
    ├── m_data (唯一缓存)
    ├── m_lock
    └── m_expireTimer

UdpAdapter ────> DataCache (注入)
TcpAdapter ────> DataCache (注入)
HttpAdapter ────> DataCache (注入)
```

**优点**：
- 数据一致性（单一缓存）
- 内存高效（唯一存储）
- 查询高效（统一接口）
- 低耦合
- 扩展性好
- 测试容易

**缺点**：
- 需要管理缓存生命周期
- 初始化复杂

---

### 3.2 数据流流程

```
1. 适配器接收原始数据
   UDP/TCP/HTTP/WS → rawDataReceived()

2. 数据解析
   parse() → PlatformData / SpecialEvent

3. 缓存存储
   DataCache::addOrUpdate()

4. 事件通知
   emit dataChanged()

5. 数据推送
   DataPublisher::pushData()
```

---

### 3.3 核心接口设计

#### IProtocolAdapter（通信层）

```cpp
class IProtocolAdapter : public QObject {
public:
    virtual ProtocolType protocolType() const = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;

signals:
    void rawDataReceived(const QByteArray &data, ProtocolType source);
    void statusChanged(AdapterStatus status);
};
```

#### IDataParser（协议层）

```cpp
class IDataParser : public QObject {
public:
    virtual bool parse(const QByteArray &rawData, ProtocolType source,
                       QList<PlatformData> &outPlatforms,
                       QList<SpecialEvent> &outEvents) = 0;
};
```

#### IDataCache（缓存层）

```cpp
class IDataCache : public QObject {
public:
    virtual bool addPlatform(const PlatformData &data) = 0;
    virtual bool updatePlatform(const PlatformData &data) = 0;
    virtual PlatformData getPlatform(const QString &id) const = 0;
    virtual QList<PlatformData> getValidPlatforms() const = 0;

signals:
    void dataChanged();
};
```

#### IDataPublisher（推送层）

```cpp
class IDataPublisher : public QObject {
public:
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual void setPushInterval(int ms) = 0;

signals:
    void dataPushed(const DynamicObjects &data);
};
```

---

## 四、关键设计决策

### 4.1 组合优于继承

**结论**：推荐使用方案 B（组合模式）

**理由**：
1. 数据一致性：单一缓存避免数据冲突
2. 内存效率：数据只存储一份
3. 可测试性：独立测试缓存和适配器
4. 扩展性：新增协议只需实现适配器
5. 符合开闭原则：对扩展开放，对修改关闭

---

### 4.2 线程安全

**策略**：在缓存层使用 `QReadWriteLock` 保护数据

**实现**：
- 读操作：`QReadLocker`
- 写操作：`QWriteLocker`
- 避免在持有锁时调用虚函数（防止死锁）

---

### 4.3 数据生命周期

**策略**：基于时间戳的过期机制

**实现**：
- 默认有效期：5000ms
- 定时器每秒检查过期数据
- 过期数据标记为 `DataStatus_Expired`
- 发射 `dataExpired` 信号通知上层

---

## 五、文件结构

```
src/
├── common/
│   ├── logger.h
│   └── logger.cpp
├── datalib/
│   ├── dynamicdata.h
│   ├── datamanager.h
│   ├── datamanager.cpp
│   ├── protocoladapter.h
│   ├── udpadapter.h
│   ├── udpadapter.cpp
│   ├── tcpadapter.h
│   ├── httpadapter.h
│   └── websocketadapter.h
├── mainwindow.h
├── mainwindow.cpp
├── viewwidget.h
├── viewwidget.cpp
└── DEVELOPMENT_NOTES.md (本文档)
```

---

## 六、测试数据

### 6.1 平台数据

| ID | Name | Longitude | Latitude | Speed | Camp |
|----|------|-----------|----------|-------|------|
| SHIP_001 | Own Ship | 121.50 | 31.20 | 12.5 | Friendly |
| SHIP_002 | Merchant A | 121.51 | 31.22 | 8.0 | Neutral |
| SHIP_003 | Fishing B | 121.48 | 31.18 | 5.0 | Neutral |
| SHIP_004 | Enemy Ship | 121.55 | 31.25 | 15.0 | Enemy |

### 6.2 示例事件

| Event ID | Name | Type | Target |
|----------|------|------|--------|
| EVENT_001 | Enemy Detected | Alert | SHIP_004 |
| EVENT_002 | Patrol Mission Started | MissionStart | SHIP_001 |

---

## 七、使用说明

### 7.1 运行程序（Qt C++ 桌面端）

```bash
cd build
cmake ..
make
./cpss.exe
```

### 7.2 操作说明

| 操作 | 说明 |
|------|------|
| 点击目标 | 显示属性框 |
| 拖动属性框 | 改变位置 |
| 双击属性框 | 关闭属性框 |
| 点击空白区域 | 隐藏所有属性框 |
| Reset 按钮 | 重置地图视图 |
| Help (F1) | 显示图例说明 |

### 7.3 快捷键

| 快捷键 | 功能 |
|--------|------|
| F1 | 显示事件图例 |
| Ctrl+R | 重置视图 |

---

## 八、Web 端项目（Vue + TypeScript）

### 8.1 技术栈

| 技术 | 版本 | 用途 |
|------|------|------|
| Vue | 3.4+ | 前端框架 |
| TypeScript | 5.4+ | 类型安全 |
| Vite | 5.2+ | 构建工具 |
| Pinia | 2.1+ | 状态管理 |

### 8.2 项目结构

```
CPSS/
├── index.html              # HTML入口
├── package.json            # 依赖配置
├── vite.config.ts          # Vite配置
├── tsconfig.json           # TypeScript配置
├── src/
│   ├── main.ts             # 应用入口
│   ├── App.vue             # 根组件
│   ├── style.css           # 全局样式
│   ├── types/
│   │   └── index.ts        # TypeScript类型定义
│   ├── stores/
│   │   ├── dataStore.ts    # 数据状态管理
│   │   └── viewStore.ts    # 视图状态管理
│   ├── components/
│   │   ├── MapCanvas.vue   # 地图画布组件
│   │   ├── PropertyBox.vue # 属性框组件
│   │   ├── ConnectingLine.vue # 连接虚线组件
│   │   ├── Toolbar.vue     # 工具栏组件
│   │   └── LegendDialog.vue # 图例对话框
│   └── mock/
│       └── data.ts         # 模拟数据
```

### 8.3 核心功能

| 功能 | 说明 |
|------|------|
| 地图展示 | Canvas绑定经纬度网格背景 |
| 平台渲染 | 根据阵营颜色区分，自有船显示船形，其他显示圆形 |
| 事件标记 | 在平台旁显示事件图标（Alert/Attack/Defense等） |
| 属性框 | 点击平台显示属性，支持拖动和双击关闭 |
| 连接虚线 | 属性框与平台之间的连接虚线 |
| 缩放平移 | 鼠标滚轮缩放，拖动平移 |
| 图例说明 | Help按钮显示事件图例和操作说明 |

### 8.4 运行方式

```bash
# 安装依赖
npm install

# 开发模式
npm run dev

# 构建生产版本
npm run build

# 预览构建结果
npm run preview
```

### 8.5 API 代理配置

```typescript
// vite.config.ts
server: {
  proxy: {
    '/api': {
      target: 'http://localhost:8080',
      changeOrigin: true
    },
    '/ws': {
      target: 'ws://localhost:8080',
      ws: true,
      changeOrigin: true
    }
  }
}
```

### 8.6 数据类型映射

| C++ 枚举 | TypeScript 枚举 | 值 |
|----------|-----------------|-----|
| CampType::Friendly | CampType.Friendly | 'friendly' |
| CampType::Enemy | CampType.Enemy | 'enemy' |
| CampType::Neutral | CampType.Neutral | 'neutral' |
| CampType::Unknown | CampType.Unknown | 'unknown' |

| C++ 枚举 | TypeScript 枚举 | 值 |
|----------|-----------------|-----|
| SpecialEventType::Alert | SpecialEventType.Alert | 'alert' |
| SpecialEventType::Attack | SpecialEventType.Attack | 'attack' |
| SpecialEventType::Defense | SpecialEventType.Defense | 'defense' |
| SpecialEventType::Contact | SpecialEventType.Contact | 'contact' |
| SpecialEventType::Damage | SpecialEventType.Damage | 'damage' |
| SpecialEventType::MissionStart | SpecialEventType.MissionStart | 'missionStart' |
| SpecialEventType::MissionEnd | SpecialEventType.MissionEnd | 'missionEnd' |
| SpecialEventType::Lost | SpecialEventType.Lost | 'lost' |
| SpecialEventType::Repair | SpecialEventType.Repair | 'repair' |
| SpecialEventType::Custom | SpecialEventType.Custom | 'custom' |

### 8.7 对比桌面端差异

| 特性 | 桌面端 (Qt) | Web端 (Vue) |
|------|-------------|-------------|
| 地图渲染 | Enclib 海图引擎 | Canvas 自定义绘制 |
| 坐标转换 | EnclTransformGeoToScrn | 自定义投影计算 |
| 属性框 | QLabel + eventFilter | Vue组件 + 事件监听 |
| 虚线连接 | QPainter::drawLine | SVG line |
| 数据管理 | QMap + QReadWriteLock | Pinia + ref |
| 事件通知 | Qt Signal/Slot | Vue reactivity |
| 部署方式 | 可执行文件 | Web浏览器 |
| 跨平台 | 需要编译 | 直接运行 |
