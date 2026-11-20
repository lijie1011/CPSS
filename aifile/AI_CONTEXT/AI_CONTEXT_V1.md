# CPSS 项目 AI 模型交接文档

> Combat Process Simulation Software - 作战过程仿真软件
> 最后更新: 2026-11-21（按实际代码校正）
> 当前版本: 1.0（与 `src/CMakeLists.txt` 一致）
> 仓库: https://github.com/lijie1011/CPSS.git

> ⚠️ **校正说明**：本文早期版本描述的 WebSocket 服务端、ChartBridge、Vue 3 前端、10 个 API 接口
> 均为设计稿，**源码中未实现**。本版已明确区分"现状"与"设计规划"。接手开发请以源码为准。

---

## 📋 项目快速上手

### 1. 项目定位
CPSS 是一个**基于 Qt 5.12 的桌面电子海图仿真程序**，当前形态是：Qt 主窗口 + Enclib 海图 SDK + DLL 插件框架。
浏览器访问 / WebSocket / Vue 3 前端属于**后续规划**，当前代码不含这些实现。

### 2. 当前实际能力
- 基于海图的桌面显示（chartplugin 封装 Enclib）
- 动态数据模型：本船 / AIS 目标 / 传感器 / 武器 / 标记
- 目标绘制、航迹、探测范围、武器轨迹、可拖动属性框
- 显示类别 / 视图分组 / 水深等高线设置面板
- 4 个 DLL 插件：chart / hello / sample / dataflow

### 3. 关键约束（按实际代码）
| 约束项 | 内容 |
|--------|------|
| Qt 版本 | 5.12.11，使用工程内随附 `src/3dParty/qt` |
| 构建工具 | CMake 3.13+，Visual Studio 2019（x64） |
| 海图引擎 | Enclib SDK（闭源），由 `cpss_enclib.zip.001~007` 解压得到 |
| 插件机制 | `PluginManager` 扫描 `bin/plugins/`，加载 `qt_plugin_instance` |
| 日志 | exe 同目录 `cpss.log` |
| 网络 | 当前无监听端口；WebSocket 为规划 |

---

## 🏗️ 技术架构（当前实际）

### 架构图（现状）
```
┌────────────────────────────────────────────────────────┐
│                    cpss.exe (Qt 5.12)                   │
│  ┌────────────────┐   ┌──────────────────────────────┐  │
│  │   MainWindow   │   │  dock 中的插件 QWidget        │  │
│  │ 工具栏/设置/面板 │   │  (DataFlow / Hello / ...)     │  │
│  └───────┬────────┘   └──────────────┬───────────────┘  │
│          │                           │                  │
│  ┌───────▼────────┐   ┌──────────────▼───────────────┐  │
│  │  DataManager   │   │  PluginManager                │  │
│  │  / DataCache   │   │  扫描并加载 bin/plugins/*.dll  │  │
│  └───────┬────────┘   └──────────────┬───────────────┘  │
│          │                           │                  │
│  ┌───────▼───────────────────────────▼───────────────┐  │
│  │            chartplugin.dll → Enclib SDK            │  │
│  │          (海图渲染 / 物标查询 / 显示类别)           │  │
│  └────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────┘
```

### 分层设计（现状）
| 层级 | 技术 | 职责 |
|------|------|------|
| 表现层 | Qt Widgets (MainWindow / 设置对话框) | 工具栏、设置面板、事件历史、平台控制 |
| 数据层 | DataManager / DataCache / 动态数据结构 | 仿真数据模型与缓存 |
| 插件层 | PluginManager + IPlugin | 扩展点，运行期加载 DLL |
| 海图引擎层 | Enclib SDK（经 chartplugin 封装） | 海图渲染、物标查询 |

> **设计规划（未实现）**：前端 Vue 3 + PixiJS，经 WebSocket 与服务端通信，详见文末"设计规划"章节。

---

## 📁 目录结构（按实际代码）

```
CPSS/
├── redme.md                        # 项目说明（已校正）
├── README.md                       # 占位
├── build_*.bat / rebuild_*.bat     # 构建/重建脚本
├── run_tests.bat                   # 测试脚本
├── pyfile/                         # 辅助脚本目录（check_dll.py / test_client.py / 图标颜色转化工具.py）
├── verify_ui.ps1                   # UI 自动化验证脚本
├── aifile/
│   ├── aireade.md
│   ├── AI_CONTEXT/AI_CONTEXT_V1.md
│   └── PROJECT_STATUS/PROJECT_STATUS_V1.md
└── src/
    ├── CMakeLists.txt              # VERSION 1.0 / CMake 3.13+
    ├── main.cpp                    # 入口（--headless / --chart-dir）
    ├── mainwindow.h/cpp/ui         # 主窗口
    ├── eventhistorydialog.h/cpp    # 事件历史
    ├── platformcontrolpanel.h/cpp  # 平台控制
    ├── displaystate.h              # 显示状态
    ├── common/                     # logger / config / xmlconfig / IconManager
    ├── datalib/                    # 数据管理与动态数据模型
    ├── plugin/                     # PluginManager + IPlugin + 4 个插件
    ├── settings/                   # 三类设置对话框
    ├── resource/                   # 图标资源
    └── 3dParty/                    # Qt + Enclib（分卷压缩包 + 解压目录）
```

### 核心文件说明（实际状态）
| 文件 | 功能 | 状态 |
|------|------|------|
| `src/CMakeLists.txt` | 构建配置、安装规则、windeployqt 部署 | ✅ |
| `src/main.cpp` | 入口，解析 `--headless` / `--chart-dir` | ✅ |
| `src/mainwindow.cpp` | 主窗口，装配面板与插件 | ✅ |
| `src/plugin/chartplugin/graphicsviewwidget.cpp` | 海图视口（运行期由 chartplugin 创建） | ✅ |
| `src/common/logger.cpp` | 日志系统，输出到 cpss.log | ✅ |
| `src/common/config.cpp` | 配置管理（cpss.ini） | ✅ |
| `src/datalib/` | 数据管理与动态数据结构 | ✅ |
| `src/plugin/` | 插件框架与 4 个插件 | ✅ |

---

## 🔧 开发环境与构建（实际）

### 环境要求
#### Windows（已验证）
| 工具 | 版本 | 说明 |
|------|------|------|
| Qt | 5.12.11 MSVC x64 | 工程内随附 `src/3dParty/qt` |
| Visual Studio | 2019 | 生成器 `Visual Studio 16 2019`，架构 x64 |
| CMake | 3.13+ | - |
| Windows SDK | 10.0.19041.0 | - |

#### Linux (Kylin V10)（未验证）
| 工具 | 版本 | 说明 |
|------|------|------|
| Qt | 5.12.x | 需自行准备 |
| GCC | 5.4+ | - |
| CMake | 3.13+ | - |
| Enclib | libenccore.so | 需从 ChartDemo 获取 |

### 首次准备（解压第三方库）
仓库只提交分卷压缩包，需先合并解压（详见 `redme.md`）：
- `cpss_enclib.zip.001~007` → `src/3dParty/Enclib/`
- `cpss_qt5.12.11.zip.001~003` → `src/3dParty/qt/`

### 构建命令
```bat
:: 推荐：直接运行脚本（已配置生成器、Qt、部署、安装）
rebuild_release.bat

:: 或手动执行
cmake -S src -B src\build -G "Visual Studio 16 2019" -A x64
cmake --build src\build --config Release
cmake --install src\build --config Release
```

### 运行命令
```bash
cpss.exe                      # GUI 模式
cpss.exe --headless           # 无 GUI
cpss.exe --chart-dir <path>   # 参数已声明但代码未使用
```
> 当前程序不监听端口，无 `--web` / `--port` 参数。

---

## 📡 设计规划（未实现，勿当作现状）

> 以下为早期设计稿，源码中**没有**任何对应实现，保留仅作后续开发参考。

### WebSocket 消息格式（设计稿）
```json
{"id": 123, "action": "getChartImage", "data": {}}
{"id": 123, "data": {}, "error": null}
{"type": "dynamicUpdate", "data": {}}
```

### 规划接口（均未实现）
| Action | 说明 | 状态 |
|--------|------|------|
| `getChartImage` | 获取海图图像 | 📋 设计中 |
| `getDynamicObjects` | 获取动态对象 | 📋 设计中 |
| `zoomAt` / `panChart` / `setScale` / `setCenter` | 视图操作 | 📋 设计中 |
| `getGeoPosition` / `queryObjects` / `getChartList` | 查询 | 📋 设计中 |
| `setDisplayCategory` | 设置显示类别 | 📋 设计中 |

### 规划中的前端（未实现）
- Vue 3 + Element Plus + PixiJS，规划端口 12345

---

## 📝 编码规范

### C++ 编码规范
```cpp
// 文件命名：小写 + 下划线；类命名：大驼峰（如 PluginManager）
// 函数/变量：小驼峰；成员前缀 m_（如 m_plugins）
// 常量：全大写 + 下划线
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
阅读 `aifile/PROJECT_STATUS/PROJECT_STATUS_V1.md`（已校正），确认功能清单实际状态。

### 第二步：环境准备
1. 使用 VS2019 + CMake 3.13+
2. 按 `redme.md` 解压两个第三方库分卷包
3. 无需单独安装 Qt（工程内随附）

### 第三步：构建
运行 `rebuild_release.bat`，产物在 `src/build/install/Release/bin/cpss.exe`。

### 第四步：开发流程
1. 修改代码后更新 `PROJECT_STATUS_V1.md`
2. 如有架构变更更新本文
3. 在变更日志中记录

---

## ⚠️ 注意事项

### 关键约束
1. **Enclib SDK 是闭源的**：不能修改，仓库只分发分卷压缩包，需手动解压。
2. **Qt 版本固定为 5.12**：使用工程内随附版本。
3. **Linux 支持未完成**：导出宏 `__declspec` 与 Enclib Linux 库分发仍需处理。
4. **WebSocket / 前端为设计稿**：不要按旧文档期待浏览器访问功能。

### 常见问题
- **CMake 报 `Enclib headers not found`**：先解压 `cpss_enclib.zip` 分卷。
- **插件不加载**：查看 `cpss.log`，确认 `bin/plugins/` 下有插件 DLL。
- **海图不显示**：检查 `3dParty/Enclib` 的 mapdata/fonts/s57data 与 lic.dat。
- **日志位置**：exe 同目录 `cpss.log`。

---

## 🔗 参考资源

### 关联项目
- ChartDemo - Enclib SDK 来源（闭源，本地路径因机器而异，不再写死）

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
