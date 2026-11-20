# CPSS 项目状态跟踪文档

> 作战过程仿真软件 | 最后更新: 2026-11-21（按实际代码校正）
> 当前版本: 1.0（与 `src/CMakeLists.txt` 一致） | 创建人: lj
> 仓库: https://github.com/lijie1011/CPSS.git

> ⚠️ 早期版本将 WebSocket 服务、部署脚本、API 接口标为"已完成"，实际源码中不存在。本版已按实际代码修正。

## 📋 项目概述

| 项目信息 | 内容 |
|---------|------|
| 项目类型 | 作战过程仿真软件（当前为 Qt 桌面程序） |
| 当前技术栈 | Qt 5.12 (C++) + Enclib 电子海图 SDK + DLL 插件 |
| 规划中 | Vue 3 前端 + WebSocket（未实现） |
| 跨平台 | Windows 已验证；Linux 有分支但未验证 |
| 关联项目 | ChartDemo（Enclib SDK 来源，闭源） |

### 当前阶段
- [x] 阶段一: 项目初始化与架构设计
- [x] 阶段二: 核心模块开发（桌面端 + 插件框架）
- [ ] 阶段三: Web 化与仿真功能完善（规划中）
- [ ] 阶段四: 测试与部署

## 📊 功能清单与进度（按实际代码）

| 功能模块 | 状态 | 说明 |
|---------|------|------|
| 项目初始化 | ✅ 已完成 | - |
| CMake 工程搭建 | ✅ 已完成 | VS2019 可用，Linux 待完善 |
| 海图引擎集成 | ✅ 已完成 | 需先解压 Enclib 分卷包 |
| 插件框架 + 4 个插件 | ✅ 已完成 | chart / hello / sample / dataflow |
| 动态数据模型与目标绘制 | ✅ 已完成 | 本船/AIS/传感器/武器/航迹/属性框 |
| 设置面板 | ✅ 已完成 | 显示类别/视图分组/水深等高线 |
| 海图视口（chartplugin） | ✅ 已完成 | GraphicsViewWidget 实现，旧 viewwidget.* 已删除 |
| WebSocket 服务 | 📋 规划中 | 未实现（原误标"已完成"） |
| 前端界面开发（Vue 3） | 📋 规划中 | web/ 目录不存在 |
| 测试与验证 | 📋 待开始 | 现有脚本引用旧路径，不可用 |

### 子功能清单

#### 1. 项目初始化
| 子功能 | 状态 | 备注 |
|--------|------|------|
| 目录结构 | ✅ | src/, common/, aifile/, 3dParty/ |
| 状态文档 | ✅ | 本文件 PROJECT_STATUS_V1.md |
| AI 交接文档 | ✅ | AI_CONTEXT/AI_CONTEXT_V1.md |
| 项目说明文档 | ✅ | redme.md |

#### 2. 架构与工程
| 子功能 | 状态 | 备注 |
|--------|------|------|
| CMake 配置 | ✅ | src/CMakeLists.txt，VERSION 1.0，CMake 3.13+ |
| Qt 5.12 集成 | ✅ | Widgets/Gui/Core/Network/Xml（无 WebSockets） |
| Enclib SDK 集成 | ✅ | 分卷压缩包 + 手动解压 |
| 构建/重建脚本 | ✅ | build_*.bat / rebuild_*.bat |
| 部署脚本 deploy.bat/sh | ❌ 不存在 | 原误标"已完成"，实际由 rebuild 内 windeployqt 完成 |

#### 3. 海图引擎集成
| 子功能 | 状态 | 备注 |
|--------|------|------|
| Enclib 初始化 | ✅ | 由 chartplugin 插件完成（非 MainWindow::init） |
| 海图加载与显示 | ✅ | chartplugin 内实现 |
| 海图操作（缩放/平移/复位） | ✅ | - |
| 物标查询 | ✅ | Enclib API 封装（非 ChartBridge 类） |
| 水深/高程设置 | ✅ | settings/waterdepthsetting |

#### 4. 数据与仿真
| 子功能 | 状态 | 备注 |
|--------|------|------|
| 动态数据模型 | ✅ | WeaponInfo / SensorInfo / PlatformData |
| 数据管理与缓存 | ✅ | DataManager / DataCache |
| 目标绘制（本船/AIS） | ✅ | - |
| 航迹 / 探测范围 / 武器轨迹 | ✅ | - |
| 可拖动属性框 | ✅ | - |
| 数据流监控插件 | ✅ | dataflowplugin |
| 仿真场景管理（保存/加载） | 📋 待开始 | - |
| 武器/传感器行为模型 | 📋 待开始 | 数据结构已有，行为待实现 |

#### 5. WebSocket 与前端（规划中，未实现）
| 子功能 | 状态 | 备注 |
|--------|------|------|
| WebSocket 服务端 | 📋 规划中 | 原误标"已完成"，源码无 WebServer 类 |
| ChartBridge 桥接类 | 📋 规划中 | 源码无此类 |
| 10 个 API 接口 | 📋 规划中 | 均为设计稿，无实现 |
| Vue 3 前端 | 📋 规划中 | web/ 目录不存在 |

#### 6. 测试与工具脚本
| 子功能 | 状态 | 备注 |
|--------|------|------|
| 单元测试 | 📋 待开始 | run_tests.bat 引用旧路径与不存在的 exe |
| DLL 检查脚本 | ✅ 已整理 | pyfile/check_dll.py，相对路径，含 chartplugin 检查 |
| 数据模拟脚本 | ⚠️ 待修复 | pyfile/test_client.py 用 UDP，与规划 WebSocket 不符 |
| UI 自动化脚本 | ✅ 已整理 | 保留 verify_ui.ps1（相对路径），删除 7 个试错版本 |

## 🎯 当前开发重点
1. ✅ 已完成：整理辅助脚本（修复旧路径、删除重复 .ps1 试错版本）
2. ✅ 已完成：移除未参与编译的旧 viewwidget.*，海图统一由 chartplugin 的 GraphicsViewWidget 实现
3. 待决策：修复代码硬伤（init() 声明、插件泄漏、授权检查等）

## 🚀 里程碑记录

| 里程碑 | 目标 | 状态 | 完成时间 |
|--------|------|------|---------|
| M1 - 项目初始化 | 创建文档和目录结构 | ✅ | 2026-07-20 |
| M2 - 工程搭建 | 可编译的 CMake 工程(Windows) | ✅ | 2026-11-20 |
| M3 - 海图集成 | 海图显示功能可用 | ✅ | 2026-11-20 |
| M4 - 基础仿真 | 动态目标绘制与数据模型 | ✅ | 2026-11-20 |
| M5 - Web 化 | WebSocket + 前端（规划） | 📋 规划中 | 待定 |
| M6 - 测试部署 | 测试通过并部署 | 📋 待开始 | 待定 |

## 📝 变更日志

| 日期 | 变更内容 | 版本 |
|------|---------|------|
| 2026-07-20 | 项目初始化，创建基础文档 | v0.1 |
| 2026-07-20 | 创建 CMake 工程，集成 Qt 5.12 和 Enclib | v0.1 |
| 2026-07-20 | 实现目标绘制、属性框等功能 | v0.1 |
| 2026-11-20 | 完成 Windows Release 构建并验证 | 1.0 |
| 2026-11-21 | 按实际代码校正全部文档，纠正虚报的完成状态 | 1.0 |

## 📁 文件清单

| 文件/目录 | 说明 |
|-----------|------|
| `redme.md` | 项目说明（已校正） |
| `aifile/AI_CONTEXT/AI_CONTEXT_V1.md` | AI 交接文档（已校正） |
| `aifile/PROJECT_STATUS/PROJECT_STATUS_V1.md` | 本文件 |
| `aifile/aireade.md` | aifile 目录说明 |
| `src/CMakeLists.txt` | CMake 构建配置（VERSION 1.0） |
| `src/main.cpp` | 程序入口 |
| `src/mainwindow.h/cpp` | 主窗口 |
| `src/plugin/chartplugin/graphicsviewwidget.cpp` | 海图视口（当前实现） |
| `src/common/` | 日志/配置/图标管理 |
| `src/datalib/` | 数据管理与动态数据 |
| `src/plugin/` | 插件框架 + 4 个插件 |
| `src/settings/` | 设置对话框 |
| `src/3dParty/` | Qt + Enclib（分卷包 + 解压目录） |

## 🔗 参考链接
- ChartDemo - Enclib SDK 来源（闭源，本地路径因机器而异，不再写死）
- Qt 5.12 文档: https://doc.qt.io/qt-5.12/
