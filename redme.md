# CPSS - Combat Process Simulation Software

> 作战过程仿真软件
> 版本: 1.0（与 `src/CMakeLists.txt` 的 `project(CPSS VERSION 1.0)` 一致）
> 创建日期: 2026-07-20
> 最近更新: 2026-11-21（按实际代码校正文档，去除未实现的设计稿）
> 创建人: lj
> git: https://github.com/lijie1011/CPSS.git

---

## 📋 项目概述

| 项目信息 | 内容 |
|---------|------|
| 项目名称 | CPSS (Combat Process Simulation Software) |
| 项目类型 | 作战过程仿真软件 |
| 当前技术栈 | Qt 5.12 (C++) + Enclib 电子海图 SDK |
| 规划中技术栈 | Vue 3 + WebSocket（**尚未实现，见下方说明**） |
| 跨平台 | Windows 已完成；Linux (Kylin V10) 代码有分支但未验证 |
| 关联项目 | ChartDemo（Enclib SDK 来源，闭源） |

> **重要说明**：本工程的**当前实际形态是一个 Qt 桌面程序**，通过 Enclib SDK 显示电子海图，
> 并通过动态库插件（DLL）扩展功能。文档早期版本中的"浏览器访问 / WebSocket 服务 /
> Vue 3 前端 / 10 个 API 接口"均为**设计规划，代码中尚未实现**，已在本版中改标为"设计中"。
> 请以 `src/` 下的源码和 `src/CMakeLists.txt` 为准。

### 核心目标
- 基于海图的作战过程仿真（桌面端已实现基础框架）
- 动态目标绘制与仿真数据展示（已实现：本船/AIS/传感器/武器/航迹）
- 通过 DLL 插件扩展功能（已实现插件框架与 4 个插件）
- 浏览器访问与 WebSocket 通信（**设计中，未实现**）

---

## 📁 当前文件结构（按实际代码）

```
CPSS/
├── redme.md                     # 本文件，项目说明（已按实际代码校正）
├── README.md                    # 占位文件
├── .gitignore                   # Git 忽略规则
├── build_debug.bat              # Debug 构建/安装脚本
├── build_release.bat            # Release 构建/安装脚本（转调 build_debug.bat Release）
├── rebuild_debug.bat            # Debug 全量重建脚本
├── rebuild_release.bat          # Release 全量重建脚本（转调 rebuild_debug.bat Release）
├── run_tests.bat                # 测试脚本（引用旧路径，待修复）
├── pyfile/                      # 辅助脚本目录
│   ├── check_dll.py             # 检查插件 DLL 元数据
│   ├── test_client.py           # 数据模拟脚本（UDP，与规划 WebSocket 不符）
│   └── 图标颜色转化工具.py       # 图标线稿转换工具
├── verify_ui.ps1                # UI 自动化验证脚本（相对路径）
├── aifile/                      # AI 记录文件目录
│   ├── aireade.md               # aifile 目录说明
│   ├── AI_CONTEXT/              # AI 交接文档存档
│   │   └── AI_CONTEXT_V1.md     # AI 模型交接文档 v1（已校正）
│   └── PROJECT_STATUS/          # 项目状态文档存档
│       └── PROJECT_STATUS_V1.md # 项目状态跟踪 v1（已校正）
└── src/                         # 服务端源代码（所有代码在此目录）
    ├── CMakeLists.txt           # CMake 构建配置（VERSION 1.0，要求 CMake 3.13+）
    ├── main.cpp                 # 程序入口（--headless / --chart-dir）
    ├── mainwindow.h/cpp/ui      # 主窗口类（含事件历史、平台控制等面板）
    ├── eventhistorydialog.h/cpp # 事件历史对话框
    ├── platformcontrolpanel.h/cpp # 平台控制面板
    ├── displaystate.h           # 显示状态定义
    ├── resource.qrc             # Qt 资源文件
    ├── common/                  # 通用模块（logger / config / xmlconfig / IconManager）
    ├── datalib/                 # 数据模块
    │   ├── datamanager.*        # 数据管理器（单例）
    │   ├── datacache.*          # 数据缓存（单例）
    │   ├── dynamicdata.h        # 数据结构定义
    │   └── comm/                # 通信适配层（各协议分目录）
    │       ├── commmanager.*    # 通信管理器，统一装配适配器
    │       ├── http/            # HTTP 适配器
    │       ├── fastdds/         # Fast DDS 适配器（含 idl/ 数据契约）
    │       ├── redis/           # Redis 适配器（订阅 AFSIM 态势数据）
    │       └── rtidds/          # RTI DDS 态势显示转发器（需 RTI SDK，未装时为桩）
    ├── plugin/                  # 插件框架 + 4 个插件（chart / hello / sample / dataflow）
    ├── settings/                # 设置对话框（显示类别 / 视图分组 / 水深等高线）
    ├── resource/                # 平台图标等资源（JB / JB_Souce 等）
    └── 3dParty/                 # 第三方库
        ├── qt/                  # 随附 Qt 5.12.11（由 zip 分卷解压，被 gitignore）
        ├── Enclib/              # Enclib SDK（由 zip 分卷解压，被 gitignore）
        ├── cpss_qt5.12.11.zip.001~003  # Qt 分卷压缩包（已提交）
        └── cpss_enclib.zip.001~007     # Enclib 分卷压缩包（已提交）
```

### 文件说明

| 文件/目录 | 说明 | 状态 |
|-----------|------|------|
| `redme.md` | 项目说明文档 | ✅ |
| `aifile/` | AI 记录文件目录 | ✅ |
| `src/CMakeLists.txt` | CMake 构建配置 | ✅ |
| `src/main.cpp` | 程序入口 | ✅ |
| `src/mainwindow.h/cpp` | 主窗口类 | ✅ |
| `src/common/` | 日志、配置、图标管理 | ✅ |
| `src/datalib/` | 数据管理与动态数据模型 | ✅ |
| `src/plugin/` | 插件框架 + 4 个插件 DLL | ✅ |
| `src/settings/` | 显示/视图/水深设置对话框 | ✅ |
| `src/3dParty/` | Qt 与 Enclib（分卷压缩包 + 解压目录） | ✅ |

---

## 🔧 开发环境要求（实际使用）

### Windows（已验证）
| 工具 | 版本 | 说明 |
|------|------|------|
| Qt | 5.12.11（MSVC x64） | 使用工程内随附的 `src/3dParty/qt`，无需单独安装 |
| Visual Studio | 2019（生成器 `Visual Studio 16 2019`，架构 x64） | 实际构建环境 |
| CMake | 3.13+ | `cmake_minimum_required(VERSION 3.13)` |
| Windows SDK | 10.0.19041.0 | 构建日志中使用 |

### Linux (Kylin V10)（有代码分支，未验证）
| 工具 | 版本 | 说明 |
|------|------|------|
| Qt | 5.12.x | 需自行准备，CMake 中 `Qt5_DIR` 仅在 WIN32 下指向随附 Qt |
| GCC | 5.4+ | - |
| CMake | 3.13+ | - |
| Enclib | `libenccore.so` | 需从 ChartDemo 获取 |
| linuxdeployqt | - | 部署用 |

> ⚠️ **Linux 构建当前不可直接通过**：`src/datalib/dynamicdata.h`、`src/common/logger.h`
> 中无条件使用了 MSVC 专有的 `__declspec(dllexport/dllimport)`，GCC 无法识别，
> 且 Linux 下的 Enclib 头文件/库未随仓库分发。跨平台分支属于待完善状态。

---

## 🚀 构建与运行

### 首次准备（重要）
仓库只提交了分卷压缩包，解压目录被 `.gitignore` 忽略。首次构建前需合并并解压：

```powershell
# PowerShell（在 src/3dParty 目录下）
Set-Location D:\CPSS\src\3dParty
cmd /c "copy /B cpss_enclib.zip.001+cpss_enclib.zip.002+cpss_enclib.zip.003+cpss_enclib.zip.004+cpss_enclib.zip.005+cpss_enclib.zip.006+cpss_enclib.zip.007 cpss_enclib.zip"
cmd /c "copy /B cpss_qt5.12.11.zip.001+cpss_qt5.12.11.zip.002+cpss_qt5.12.11.zip.003 cpss_qt5.12.11.zip"
Expand-Archive -Path cpss_enclib.zip -DestinationPath . -Force
Expand-Archive -Path cpss_qt5.12.11.zip -DestinationPath . -Force
```

解压后应存在 `src/3dParty/Enclib/include/encl.h` 和 `src/3dParty/qt/lib/cmake/Qt5/Qt5Config.cmake`。

### Windows 构建（推荐）
直接运行根目录下的批处理脚本（内部已配置好生成器、Qt、部署与安装）：

```bat
rebuild_release.bat    :: 全量重建 Release（清理 → configure → build → windeployqt → install）
rebuild_debug.bat      :: 全量重建 Debug
build_release.bat      :: 增量构建 Release
build_debug.bat        :: 增量构建 Debug
```

产物位置：
- 直接运行：`src\build\bin\Release\cpss.exe`
- 安装版本：`src\build\install\Release\bin\cpss.exe`（含 Qt 运行时与 Enclib 数据，推荐）
- 构建日志：`build_log.txt`（仅 rebuild 脚本生成，被 `.gitignore` 忽略）

### 手动 CMake 命令（等价于脚本内部步骤）
```bat
cmake -S src -B src\build -G "Visual Studio 16 2019" -A x64
cmake --build src\build --config Release
cmake --install src\build --config Release
```

### 运行模式（实际支持的命令行参数）
```bash
cpss.exe                      # 桌面 GUI 模式（默认）
cpss.exe --headless           # 无 GUI 模式（仅 QCoreApplication，不创建主窗口）
cpss.exe --chart-dir <path>   # 声明了此参数（当前代码未使用）
```
> 说明：当前程序**不监听任何网络端口**，`--web` / `--port` 参数不存在。
> 日志输出到 exe 同目录的 `cpss.log`。

### AFSIM 态势数据接入（Redis）
CPSS 通过 Redis 订阅 AFSIM 仿真软件发布的态势数据，链路为：

```
AFSIM redis_ext (sendCustomize_Info)
   └─ Redis PUBLISH "target:data:channel" (HTProto::Customize_Info, protobuf)
        └─ CPSS RedisAdapter 订阅 → PlatformData → DataManager → 海图显示
                                          └→ SituationForwarder → 态势显示（RTI DDS，需SDK）
```

**配置**（exe 同目录 `cpss.ini`）：
```ini
[Redis]
Host=127.0.0.1
Port=6379
```
默认值即 `127.0.0.1:6379`。订阅频道固定为 `target:data:channel`。

**测试数据开关**（默认关闭，海图只显示 Redis 真实数据）：
```ini
[DataSource]
UseTestData=false    ; 默认 false；设为 true 可开启本地模拟目标（调试用）
```
> 之前海图上自动出现的"红船1/红船2"等目标来自 `ChartPlugin` 启动的本地测试数据定时器，
> 现已默认关闭。需要恢复模拟数据时才设为 `true`。

**重要约束**：
- Redis SDK（`3dParty/redis_sdk`）只有 **Release(/MD)** 版。**必须用 Release 配置运行**才能接收数据；
  Debug(/MDd) 下 RedisAdapter 自动降级为桩（不崩溃，但不接收数据），日志会提示用 Release。
- Redis SDK 的 DLL（含 protobuf/hiredis/zlib）已配置 POST_BUILD 自动部署到 exe 目录。
- 态势显示转发（RTI Connext DDS）为商业库，未装 SDK 时为空桩；装了 SDK 并定义 `CPSS_ENABLE_RTI_DDS` 后启用。

---

## 📊 功能清单（按实际代码状态）

| 功能模块 | 状态 | 说明 |
|---------|------|------|
| 项目初始化 | ✅ 已完成 | - |
| CMake 工程搭建 | ✅ 已完成 | Windows(VS2019) 可用，Linux 分支待完善 |
| 海图引擎集成 | ✅ 已完成 | 需先解压 Enclib 分卷包 |
| 插件框架 + 4 个插件 | ✅ 已完成 | chart / hello / sample / dataflow |
| 动态数据模型与目标绘制 | ✅ 已完成 | 本船/AIS/传感器/武器/航迹/属性框 |
| 设置面板（显示类别/视图分组/水深） | ✅ 已完成 | - |
| WebSocket 服务 | 📋 设计中 | **未实现**（原文档误标"已完成"） |
| 前端界面开发（Vue 3） | 📋 设计中 | 未开始，`web/` 目录不存在 |
| 浏览器访问 / API 接口 | 📋 设计中 | 未实现 |
| 测试与验证 | 📋 待开始 | 现有脚本引用旧路径，不可用 |

---

## 🏗️ 技术架构（当前实际）

### 桌面端架构
```
┌─────────────────────────────────────────────┐
│              cpss.exe (Qt 5.12)             │
│  ┌───────────────┐  ┌──────────────────┐    │
│  │  MainWindow   │  │  dock 中的插件控件 │    │
│  │  工具栏/设置   │  │  (QWidget)        │    │
│  └───────┬───────┘  └────────┬─────────┘    │
│          │                   │              │
│  ┌───────▼───────┐  ┌────────▼─────────┐    │
│  │  DataManager  │  │  PluginManager    │    │
│  │  /DataCache   │  │  (扫描 plugins/)  │    │
│  └───────┬───────┘  └────────┬─────────┘    │
│          │                   │              │
│  ┌───────▼───────────────────▼─────────┐    │
│  │  chartplugin.dll → Enclib SDK       │    │
│  │  (电子海图渲染 / 物标查询)           │    │
│  └─────────────────────────────────────┘    │
└─────────────────────────────────────────────┘
```

### 模块架构（实际参与编译的源码）
```
src/
├── main.cpp                 # 程序入口，命令行参数解析
├── mainwindow.cpp           # 主窗口，装配各面板与插件
├── eventhistorydialog.cpp   # 事件历史
├── platformcontrolpanel.cpp # 平台控制
├── settings/                # 显示类别 / 视图分组 / 水深设置
├── common/                  # logger / config / xmlconfig / IconManager
├── datalib/                 # DataManager / DataCache / 动态数据结构
├── plugin/                  # PluginManager + IPlugin 接口
│   └── chartplugin/         # 海图插件（封装 Enclib）
│   └── helloplugin/         # 示例插件
│   └── sampleplugin/        # 示例插件
│   └── dataflowplugin/      # 数据流监控插件
└── 3dParty/                 # Qt + Enclib
```

---

## 📡 API 接口（设计中，尚未实现）

> 以下接口为早期设计稿，**源码中没有任何对应的服务端实现**，保留仅作规划参考。

### WebSocket 消息格式（设计稿）
```json
// 请求
{"id": 123, "action": "getChartImage", "data": {}}

// 响应
{"id": 123, "data": {}, "error": null}

// 推送（100ms）
{"type": "dynamicUpdate", "data": {}}
```

### 规划接口（均未实现）
| 接口 | 功能 | 状态 |
|------|------|------|
| `getChartImage` | 获取海图图像 | 📋 设计中 |
| `getDynamicObjects` | 获取动态对象 | 📋 设计中 |
| `zoomAt` / `panChart` / `setScale` / `setCenter` | 视图操作 | 📋 设计中 |
| `getGeoPosition` / `queryObjects` / `getChartList` | 查询 | 📋 设计中 |
| `setDisplayCategory` | 设置显示类别 | 📋 设计中 |

---

## 📝 文档说明

### 核心文档

| 文档 | 位置 | 说明 |
|------|------|------|
| AI 交接文档 | `aifile/AI_CONTEXT/AI_CONTEXT_V1.md` | AI 模型接手项目必看（已按实际代码校正） |
| 项目状态文档 | `aifile/PROJECT_STATUS/PROJECT_STATUS_V1.md` | 功能进度跟踪（已校正） |
| aifile 说明 | `aifile/aireade.md` | AI 记录文件管理规范 |

### AI 记录文件规范
- AI 记录文件存放在 `aifile/` 目录下
- 每次更新项目状态时，在相应文件夹中记录更新内容
- 文件名格式：`AI_CONTEXT_V{n}.md` / `PROJECT_STATUS_V{n}.md`

---

## ⚠️ 注意事项

### 关键约束
1. **Enclib SDK 是闭源的**：不能修改，只能调用 API；仓库只分发分卷压缩包，需手动解压。
2. **Qt 版本固定为 5.12**：使用工程内随附版本，CMake 中 `Qt5_DIR` 指向 `src/3dParty/qt`。
3. **Linux 支持未完成**：导出宏 `__declspec` 与 Enclib Linux 库分发仍需处理。
4. **WebSocket / 前端为设计稿**：不要按旧文档期待浏览器访问功能。

### 常见问题
- **CMake 报 `Enclib headers not found`**：先按"首次准备"解压 `cpss_enclib.zip` 分卷。
- **程序缺少 Qt DLL**：运行 `cmake --install` 或 rebuild 脚本，由 windeployqt 部署运行时。
- **插件不加载**：查看 `cpss.log`，确认 `bin/plugins/` 下存在插件 DLL。
- **海图不显示**：检查 `3dParty/Enclib` 的 mapdata/fonts/s57data 是否齐全、lic.dat 是否有效。

---

## 🔗 参考资源

### 关联项目
- ChartDemo - 海图引擎基础 / Enclib SDK 来源（闭源，本地路径因机器而异，不再写死）

### 技术文档
- [Qt 5.12 文档](https://doc.qt.io/qt-5.12/)
- Enclib SDK 文档（闭源，随 SDK 分发）

---

## 📞 联系方式

| 信息 | 内容 |
|------|------|
| 创建人 | lj |
| 创建日期 | 2026-07-20 |
| 当前版本 | 1.0 |
| 更新频率 | 每次开发完成后更新 |
