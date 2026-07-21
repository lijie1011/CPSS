# CPSS - Combat Process Simulation Software

> 作战过程仿真软件
> 版本: v0.1
> 创建日期: 2026-07-20
> 创建人: lj
> git:

---

## 📋 项目概述

| 项目信息 | 内容 |
|---------|------|
| 项目名称 | CPSS (Combat Process Simulation Software) |
| 项目类型 | 作战过程仿真软件 |
| 技术栈 | Qt 5.12 (C++) + Vue 3 (TypeScript) + WebSocket |
| 跨平台 | Windows 10/11 + Linux (Kylin V10) |
| 关联项目 | [ChartDemo](file:///d:/ZCHH/ChartDemo) - 海图引擎基础 |

### 核心目标
- 基于海图的作战过程仿真
- 支持浏览器访问（WebSocket 通信）
- 跨平台部署（Windows + Linux）
- 100ms 高频动态刷新

---

## 📁 当前文件结构

```
CPSS/
├── redme.md                     # 本文件，项目说明
├── aifile/                      # AI 记录文件目录
│   ├── aireade.md               # aifile 目录说明
│   ├── AI_CONTEXT/              # AI 交接文档存档
│   │   └── AI_CONTEXT_V1.md     # AI 模型交接文档 v1
│   └── PROJECT_STATUS/          # 项目状态文档存档
│       └── PROJECT_STATUS_V1.md # 项目状态跟踪 v1
├── src/                         # 服务端源代码（所有代码在此目录）
│   ├── CMakeLists.txt           # CMake 构建配置
│   ├── main.cpp                 # 程序入口
│   ├── mainwindow.h/cpp         # 主窗口类
│   ├── viewwidget.h/cpp         # 海图视口类
│   ├── webserver.h/cpp          # WebSocket 服务端
│   ├── chartbridge.h/cpp        # 海图桥接 API
│   ├── common/                  # 通用模块
│   │   ├── logger.h/cpp         # 日志系统
│   │   └── config.h/cpp         # 配置管理
│   └── 3dParty/                 # 第三方库
│       └── Enclib/              # Enclib SDK（电子海图引擎）
└── (待创建: web/)               # 前端代码目录
```

### 文件说明

| 文件/目录 | 说明 | 状态 |
|-----------|------|------|
| `redme.md` | 项目说明文档 | ✅ |
| `aifile/` | AI 记录文件目录 | ✅ |
| `aifile/AI_CONTEXT/` | AI 交接文档存档 | ✅ |
| `aifile/PROJECT_STATUS/` | 项目状态文档存档 | ✅ |
| `src/` | 服务端源代码目录（所有代码在此） | ✅ |
| `src/CMakeLists.txt` | CMake 构建配置 | ✅ |
| `src/main.cpp` | 程序入口 | ✅ |
| `src/mainwindow.h/cpp` | 主窗口类 | ✅ |
| `src/viewwidget.h/cpp` | 海图视口类 | ✅ |
| `src/webserver.h/cpp` | WebSocket 服务端 | ✅ |
| `src/chartbridge.h/cpp` | 海图桥接 API | ✅ |
| `src/common/logger.h/cpp` | 日志系统 | ✅ |
| `src/common/config.h/cpp` | 配置管理 | ✅ |
| `src/3dParty/Enclib/` | Enclib SDK | ✅ |
| `web/` | 前端代码目录 | ❌ 待创建 |

---

## 🔧 开发环境要求

### Windows
| 工具 | 版本 | 路径示例 |
|------|------|---------|
| Qt | 5.12.11 | `C:\Qt\5.12.11\msvc2017_64` |
| Visual Studio | 2017 | `C:\Program Files (x86)\Microsoft Visual Studio\2017\Community` |
| CMake | 3.10+ | - |

### Linux (Kylin V10)
| 工具 | 版本 | 路径示例 |
|------|------|---------|
| Qt | 5.12.x | `/opt/Qt/5.12.11/gcc_64` |
| GCC | 5.4+ | - |
| CMake | 3.10+ | - |
| linuxdeployqt | - | - |

---

## 🚀 构建与运行

### Windows 构建
```bat
set Qt5_DIR=C:\Qt\5.12.11\msvc2017_64
mkdir build_windows
cd build_windows
cmake -G "Visual Studio 15 2017 Win64" ../src
cmake --build . --config Release
```

### Linux 构建
```bash
export Qt5_DIR=/opt/Qt/5.12.11/gcc_64
export CMAKE_PREFIX_PATH=$Qt5_DIR
export LD_LIBRARY_PATH=$Qt5_DIR/lib:$LD_LIBRARY_PATH

mkdir -p build_linux
cd build_linux
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../src
make -j$(nproc)
```

### 运行模式
```bash
# 桌面模式（带 GUI）
cpss.exe

# Web 服务模式（同时启动 WebSocket）
cpss.exe --web --port 12345

# 无界面服务模式（Linux 服务器）
cpss --headless --web --port 12345
```

---

## 📊 功能清单

### 核心功能模块

| 功能模块 | 状态 | 优先级 | 预计完成时间 |
|---------|------|--------|-------------|
| 项目初始化 | ✅ 已完成 | 高 | 2026-07-20 |
| 技术架构设计 | ✅ 已完成 | 高 | 2026-07-20 |
| CMake 工程搭建 | ✅ 已完成 | 高 | 2026-07-20 |
| 海图引擎集成 | ✅ 已完成 | 高 | 2026-07-20 |
| WebSocket 服务 | ✅ 已完成 | 高 | 2026-07-20 |
| 前端界面开发 | 📋 待开始 | 中 | 2026-09-01 |
| 作战仿真核心 | 📋 待开始 | 高 | 2026-09-15 |
| 数据可视化 | 📋 待开始 | 中 | 2026-09-30 |
| 测试与验证 | 📋 待开始 | 高 | 2026-11-15 |

---

## 🏗️ 技术架构

### 整体架构
```
┌─────────────────────┐     WebSocket     ┌─────────────────────┐
│   浏览器端 (Vue 3)   │ ←────────────────→ │   Qt 5.12 服务端    │
│   Vue + PixiJS      │   双向通信         │   C++ + Enclib      │
│                     │                    │                     │
│  UI组件层           │                    │  WebSocket Server   │
│  PixiJS渲染层       │                    │  ChartWebBridge     │
│                     │                    │  Enclib SDK         │
└─────────────────────┘                    └─────────────────────┘
```

### 技术选型

| 层级 | 技术 | 职责 |
|------|------|------|
| 前端 UI | Vue 3 + Element Plus | 工具栏、设置面板 |
| 前端渲染 | PixiJS (WebGL) | 海图显示、动态对象 |
| 通信层 | WebSocket | 双向数据传输 |
| 服务端 | Qt 5.12 (C++) | 海图操作、仿真逻辑 |
| 海图引擎 | Enclib SDK | 海图渲染、物标查询 |

### 模块架构

```
src/
├── main.cpp                 # 程序入口，命令行参数解析
├── mainwindow.h/cpp         # 主窗口，初始化 Enclib
├── viewwidget.h/cpp         # 海图视口，渲染和交互
├── webserver.h/cpp          # WebSocket 服务端
├── chartbridge.h/cpp        # 海图 API 桥接
└── common/
    ├── logger.h/cpp         # 日志系统
    └── config.h/cpp         # 配置管理
```

---

## 📡 API 接口设计

### WebSocket 消息格式
```json
// 请求
{"id": 123, "action": "getChartImage", "data": {}}

// 响应
{"id": 123, "data": {}, "error": null}

// 推送（100ms）
{"type": "dynamicUpdate", "data": {}}
```

### 已实现的接口
| 接口 | 功能 | 参数 | 返回值 |
|------|------|------|--------|
| `getChartImage` | 获取海图图像 | 无 | base64 编码的 WebP 图像 |
| `getDynamicObjects` | 获取动态对象 | 无 | 本船、AIS 目标等 |
| `zoomAt` | 指定位置缩放 | x, y, factor | success, scale |
| `panChart` | 平移海图 | fromX, fromY, toX, toY | success |
| `setScale` | 设置比例尺 | scale | success |
| `setCenter` | 设置中心点 | lon, lat | success |
| `getGeoPosition` | 屏幕坐标转经纬度 | x, y | lon, lat |
| `queryObjects` | 查询物标 | x, y, radius | objects, count |
| `getChartList` | 获取海图列表 | 无 | charts, count |
| `setDisplayCategory` | 设置显示类别 | category | success |

---

## 📝 文档说明

### 核心文档

| 文档 | 位置 | 说明 |
|------|------|------|
| AI 交接文档 | `aifile/AI_CONTEXT/AI_CONTEXT_V1.md` | AI 模型接手项目必看 |
| 项目状态文档 | `aifile/PROJECT_STATUS/PROJECT_STATUS_V1.md` | 功能进度跟踪 |
| aifile 说明 | `aifile/aireade.md` | AI 记录文件管理规范 |

### AI 记录文件规范
- AI 记录文件存放在 `aifile/` 目录下
- 每次更新项目状态时，在相应文件夹中记录更新内容
- 文件名格式：`AI_CONTEXT_V{n}.md` / `PROJECT_STATUS_V{n}.md`

---

## ⚠️ 注意事项

### 关键约束
1. **Enclib SDK 是闭源的**：不能修改，只能调用 API
2. **Qt 版本固定为 5.12**：不能升级
3. **Linux 版本需要 libenccore.so**：已从 ChartDemo 复制
4. **100ms 刷新要求**：动态层必须用 JSON 推送

### 常见问题
- **Windows 编译失败**：检查 Qt 路径是否正确
- **Linux 运行缺少库**：设置 LD_LIBRARY_PATH
- **WebSocket 连接失败**：检查端口是否被占用
- **海图不显示**：检查 Enclib 初始化路径

---

## 🔗 参考资源

### 关联项目
- [ChartDemo](file:///d:/ZCHH/ChartDemo) - 海图引擎基础
- [Enclib SDK](file:///d:/ZCHH/CPSS/src/3dParty/Enclib) - 电子海图 SDK

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
