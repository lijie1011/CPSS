# CPSS 项目 AI 模型交接文档

> Combat Process Simulation Software - 作战过程仿真软件
> 最后更新: 2026-07-20
> 当前版本: v0.1

---

## 📋 项目快速上手

### 1. 项目定位
CPSS 是一个**作战过程仿真软件**，基于 Qt 5.12 开发，支持浏览器访问和本地部署，跨平台运行在 Windows 和 Linux。

### 2. 核心目标
- 基于海图的作战过程仿真
- 支持浏览器访问（WebSocket 通信）
- 跨平台部署（Windows + Linux）
- 100ms 高频动态刷新

### 3. 关键约束
| 约束项 | 内容 |
|--------|------|
| Qt 版本 | 5.12（固定） |
| 海图引擎 | Enclib SDK（闭源，已有 Linux 版） |
| 前端技术 | Vue 3 + TypeScript + PixiJS |
| 通信方式 | WebSocket（双向通信） |
| 刷新频率 | 动态层 100ms |
| 跨平台 | Windows 10/11 + Linux Kylin V10 |

---

## 🏗️ 技术架构

### 整体架构图

```
┌────────────────────────────────────────────────────────────────┐
│                      CPSS 系统架构                             │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌─────────────────────┐                                       │
│  │    浏览器端 (Vue 3)  │                                       │
│  │                     │                                       │
│  │  ┌───────────────┐  │                                       │
│  │  │   UI 组件层   │  │  Vue 3 + Element Plus                  │
│  │  │  Toolbar/List │  │                                       │
│  │  └───────┬───────┘  │                                       │
│  │          │          │                                       │
│  │  ┌───────▼───────┐  │                                       │
│  │  │  PixiJS 渲染层 │  │  WebGL 加速，60 FPS                   │
│  │  │  静态层+动态层 │  │                                       │
│  │  └───────┬───────┘  │                                       │
│  │          │ WebSocket│                                       │
│  │          ▼          │                                       │
│  │  ┌───────────────┐  │                                       │
│  │  │  WebSocket客户端│  │  双向通信，JSON 协议                   │
│  │  └───────────────┘  │                                       │
│  └───────────┬─────────┘                                       │
│              │ WebSocket (ws://localhost:12345)                 │
│              ▼                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Qt 5.12 服务端 (C++)                        │   │
│  │                                                         │   │
│  │  ┌───────────────┐  ┌───────────────────────────────┐   │   │
│  │  │   WebServer   │  │       ChartBridge             │   │   │
│  │  │  WebSocket    │←→│  - getChartImage（按需）      │   │   │
│  │  │  Server       │  │  - getDynamicObjects(100ms)   │   │   │
│  │  └───────────────┘  │  - 交互指令处理               │   │   │
│  │                     └───────────────────────────────┘   │   │
│  │                                      │                   │   │
│  │              ┌───────────────────────┴───────────────┐   │   │
│  │              │              Enclib SDK               │   │   │
│  │              │    电子海图渲染引擎 (闭源)             │   │   │
│  │              └───────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### 分层设计

| 层级 | 技术 | 职责 |
|------|------|------|
| **前端 UI 层** | Vue 3 + Element Plus | 工具栏、设置面板、信息展示 |
| **前端渲染层** | PixiJS (WebGL) | 海图显示、动态对象渲染 |
| **通信层** | WebSocket | 双向数据传输 |
| **服务端业务层** | Qt 5.12 (C++) | 海图操作、仿真逻辑 |
| **海图引擎层** | Enclib SDK | 海图渲染、物标查询 |

---

## 📁 目录结构

```
CPSS/
├── redme.md               # 项目说明文档
├── aifile/                # AI 记录文件目录
│   ├── AI_CONTEXT/        # AI 交接文档存档
│   │   └── AI_CONTEXT_V1.md
│   └── PROJECT_STATUS/    # 项目状态文档存档
│       └── PROJECT_STATUS_V1.md
├── src/                   # 服务端源代码（所有代码在此）
│   ├── CMakeLists.txt     # CMake 配置
│   ├── main.cpp           # 程序入口，命令行参数解析
│   ├── mainwindow.h/cpp   # 主窗口，初始化 Enclib
│   ├── viewwidget.h/cpp   # 海图视口，渲染和交互
│   ├── webserver.h/cpp    # WebSocket 服务端
│   ├── chartbridge.h/cpp  # 海图 API 桥接
│   ├── common/            # 通用工具
│   │   ├── logger.h/cpp   # 日志系统
│   │   └── config.h/cpp   # 配置管理
│   └── 3dParty/           # 第三方库
│       └── Enclib/        # 海图 SDK
└── web/                   # 前端代码（待创建）
```

### 核心文件说明

| 文件 | 功能 | 状态 |
|------|------|------|
| `src/CMakeLists.txt` | CMake 构建配置，跨平台编译和部署脚本 | ✅ |
| `src/main.cpp` | 程序入口，解析命令行参数（--web, --port, --headless） | ✅ |
| `src/mainwindow.h/cpp` | 主窗口类，初始化 Enclib SDK，创建工具栏 | ✅ |
| `src/viewwidget.h/cpp` | 海图视口类，处理渲染、缩放、平移、鼠标交互 | ✅ |
| `src/webserver.h/cpp` | WebSocket 服务端，管理客户端连接和消息处理 | ✅ |
| `src/chartbridge.h/cpp` | 海图桥接类，封装 Enclib API，处理 WebSocket 请求 | ✅ |
| `src/common/logger.h/cpp` | 日志系统，支持 DEBUG/INFO/WARN/ERROR 级别 | ✅ |
| `src/common/config.h/cpp` | 配置管理，读取/保存 cpss.ini 配置文件 | ✅ |

---

## 🔧 开发环境与构建

### 环境要求

#### Windows
| 工具 | 版本 | 路径示例 |
|------|------|---------|
| Qt | 5.12.11 | `C:\Qt\5.12.11\msvc2017_64` |
| Visual Studio | 2017 | `C:\Program Files (x86)\Microsoft Visual Studio\2017\Community` |
| CMake | 3.10+ | - |

#### Linux (Kylin V10)
| 工具 | 版本 | 路径示例 |
|------|------|---------|
| Qt | 5.12.x | `/opt/Qt/5.12.11/gcc_64` |
| GCC | 5.4+ | - |
| CMake | 3.10+ | - |
| linuxdeployqt | - | - |

### 构建命令

#### Windows 构建
```bat
set Qt5_DIR=C:\Qt\5.12.11\msvc2017_64
mkdir build_windows
cd build_windows
cmake -G "Visual Studio 15 2017 Win64" ../src
cmake --build . --config Release
```

#### Linux 构建
```bash
export Qt5_DIR=/opt/Qt/5.12.11/gcc_64
export CMAKE_PREFIX_PATH=$Qt5_DIR
export LD_LIBRARY_PATH=$Qt5_DIR/lib:$LD_LIBRARY_PATH

mkdir -p build_linux
cd build_linux
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../src
make -j$(nproc)
```

#### 运行命令
```bash
# 桌面模式（带 GUI）
cpss.exe

# Web 服务模式（同时启动 WebSocket）
cpss.exe --web --port 12345

# 无界面服务模式（Linux 服务器）
cpss --headless --web --port 12345
```

---

## 📡 API 接口设计

### WebSocket 消息格式

```json
// 请求格式
{
  "id": 123,
  "action": "getChartImage",
  "data": {}
}

// 响应格式
{
  "id": 123,
  "data": {},
  "error": null
}

// 推送格式
{
  "type": "dynamicUpdate",
  "data": {}
}
```

### 已实现的接口

| Action | 说明 | 请求参数 | 响应数据 |
|--------|------|---------|---------|
| `getChartImage` | 获取海图图像 | 无 | `{ image, width, height, format }` |
| `getDynamicObjects` | 获取动态对象 | 无 | `{ ship, timestamp, aisTargets, weapons, sensors }` |
| `zoomAt` | 指定位置缩放 | `{ x, y, factor }` | `{ success, scale }` |
| `panChart` | 平移海图 | `{ fromX, fromY, toX, toY }` | `{ success }` |
| `setScale` | 设置比例尺 | `{ scale }` | `{ success }` |
| `setCenter` | 设置中心点 | `{ lon, lat }` | `{ success }` |
| `getGeoPosition` | 屏幕坐标转经纬度 | `{ x, y }` | `{ lon, lat }` |
| `queryObjects` | 查询物标 | `{ x, y, radius }` | `{ objects, count }` |
| `getChartList` | 获取海图列表 | 无 | `{ charts, count }` |
| `setDisplayCategory` | 设置显示类别 | `{ category }` | `{ success }` |

### 动态推送数据格式（100ms）

```json
{
  "type": "dynamicUpdate",
  "data": {
    "timestamp": 1234567890,
    "ship": {
      "lon": 121.5,
      "lat": 31.2,
      "x": 500,
      "y": 350,
      "heading": 45.0,
      "speed": 12.5
    },
    "aisTargets": [],
    "weapons": [],
    "sensors": []
  }
}
```

---

## 📝 编码规范

### C++ 编码规范

```cpp
// 文件命名：小写 + 下划线
// 头文件：xxx.h
// 源文件：xxx.cpp

// 类命名：大驼峰
class WebServer;
class ChartBridge;

// 函数命名：小驼峰
void onNewConnection();
QJsonObject getChartImage();

// 变量命名：小驼峰
QWebSocketServer *m_server;
QList<QWebSocket*> m_clients;

// 常量命名：全大写 + 下划线
const int DEFAULT_PORT = 12345;
const QString DEFAULT_CHART_DIR = "../3dParty/Enclib";

// 宏定义：全大写 + 下划线
#define OS_WINDOWS 1
#define OS_LINUX 1
```

### TypeScript 编码规范

```typescript
// 文件命名：小写 + 下划线
// 组件：PascalCase.vue
// 组合式函数：useXxx.ts
// 状态管理：xxxStore.ts

// 类命名：大驼峰
class WebSocketClient {}

// 函数命名：小驼峰
function sendMessage(action: string, data?: any): Promise<any> {}

// 变量命名：小驼峰
const wsUrl = 'ws://localhost:12345';
let isConnected = false;

// 常量命名：全大写 + 下划线
const DEFAULT_PORT = 12345;

// 类型定义：大驼峰
interface ChartData {
  image: string;
  width: number;
  height: number;
}
```

### Git 提交规范

```
<type>(<scope>): <subject>

<body>

<footer>
```

| Type | 说明 |
|------|------|
| `feat` | 新功能 |
| `fix` | 修复 Bug |
| `docs` | 文档更新 |
| `style` | 代码格式 |
| `refactor` | 重构 |
| `test` | 测试 |
| `chore` | 构建/工具 |

---

## 🎯 如何接手开发

### 第一步：了解项目状态
1. 阅读 `aifile/PROJECT_STATUS/PROJECT_STATUS_V1.md` 了解当前进度
2. 查看功能清单中的状态标记
3. 确认当前开发重点

### 第二步：环境准备
1. 安装 Qt 5.12（Windows/Linux）
2. 配置 CMake
3. Enclib SDK 已复制到 `3dParty/Enclib`

### 第三步：构建项目
1. 创建 build 目录
2. 运行 CMake
3. 编译项目
4. 运行测试

### 第四步：开发流程
1. 更新 `PROJECT_STATUS_V1.md` 中的功能状态
2. 编写代码
3. 测试代码
4. 更新变更日志

### 第五步：交接给下一个 AI
1. 更新 `PROJECT_STATUS_V1.md`
2. 更新 `AI_CONTEXT_V1.md`（如有架构变更）
3. 在变更日志中记录

---

## ⚠️ 注意事项

### 关键约束
1. **Enclib SDK 是闭源的**：不能修改，只能调用 API
2. **Qt 版本固定为 5.12**：不能升级
3. **Linux 版本需要 libenccore.so**：已从 ChartDemo 复制
4. **100ms 刷新要求**：动态层必须用 JSON 推送，不能传图像

### 常见问题
1. **Windows 编译失败**：检查 Qt 路径是否正确
2. **Linux 运行缺少库**：设置 LD_LIBRARY_PATH
3. **WebSocket 连接失败**：检查端口是否被占用
4. **海图不显示**：检查 Enclib 初始化路径

### 调试技巧
1. 使用 Qt Creator 的调试功能
2. WebSocket 调试使用浏览器开发者工具
3. 日志输出到 `cpss.log` 文件
4. 前端使用 Vue DevTools

---

## 🔗 参考资源

### 核心文件
- [PROJECT_STATUS_V1.md](file:///d:/ZCHH/CPSS/aifile/PROJECT_STATUS/PROJECT_STATUS_V1.md) - 项目状态跟踪
- [redme.md](file:///d:/ZCHH/CPSS/redme.md) - 项目说明文档

### 关联项目
- [ChartDemo](file:///d:/ZCHH/ChartDemo) - 海图引擎基础
- [Enclib SDK](file:///d:/ZCHH/CPSS/3dParty/Enclib) - 电子海图 SDK

### 技术文档
- [Qt 5.12 文档](https://doc.qt.io/qt-5.12/)
- [Vue 3 文档](https://vuejs.org/)
- [PixiJS 文档](https://pixijs.com/)
- [Element Plus 文档](https://element-plus.org/)

---

## 📞 联系方式

| 信息 | 内容 |
|------|------|
| 创建人 | lj |
| 创建日期 | 2026-07-20 |
| 当前版本 | v0.1 |
| 更新频率 | 每次开发完成后更新 |
