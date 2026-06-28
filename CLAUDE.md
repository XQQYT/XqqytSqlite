# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 硬性要求

阅读"G:\XqqytSqlite\计划书.md"和"G:\XqqytSqlite\技术选型.md"

## 项目概述

**RemoteSQLite-Qt** — 基于 Qt 6.8 (QML + C++) 的远程 SQLite 管理工具。通过 SSH 在远端执行 `sqlite3` 命令操作数据库，并支持 SFTP 双向同步（下载到本地操作后上传回设备）。

## 构建命令

项目使用 CMake 3.27+ + vcpkg manifest mode。

```bash
# 克隆（必须带 submodule）
git clone --recurse-submodules <repo-url>

# 配置（Windows + MSVC）
cmake --preset win-msvc-release

# 配置（Linux + GCC）
cmake --preset linux-gcc-release

# 编译
cmake --build --preset win-msvc-release --config Release

# 运行 clang-tidy（Windows 需先通过 pip 安装 clang-tidy）
clang-tidy -config-file=.clang-tidy src/**/*.cpp -- -std=c++17

# 格式化检查
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h

# 格式化应用
clang-format -i src/**/*.cpp src/**/*.h

# 运行测试
ctest --preset default

# 打包 — Windows 便携 ZIP
cmake --package --preset win-portable-zip

# 打包 — Windows NSIS 安装包（需安装 NSIS）
cmake --package --preset win-nsis-installer

# 打包 — Windows 两种格式
cmake --package --preset win-all

# 打包 — Linux DEB + TGZ
cmake --package --preset linux-deb-tgz

# 一键打包脚本 (Windows PowerShell)
# 自动完成: MSVC Release 编译 → windeployqt 收集 Qt DLL
#           → 复制 vcpkg 运行时 DLL → CPack 生成 ZIP/NSIS
.\scripts\package.ps1                # 两种格式
.\scripts\package.ps1 -Portable      # 仅 ZIP
.\scripts\package.ps1 -Installer     # 仅 NSIS
.\scripts\package.ps1 -Clean         # 清理后重新打包
```

## 分层架构（核心约束）

架构严格自上而下依赖，**严禁跨层调用**：

```
QML 层 (Qt Quick)           ← UI 渲染，仅此层写 QML/JS
  └── Model 层 (C++ + Qt)   ← QAbstractListModel/QAbstractTableModel，通过信号槽与 QML 绑定
        └── EventBus 层      ← XQQYT/EventBus git submodule，纯 C++17，仅 STL
              └── Controller 层 ← 纯 C++17，业务逻辑编排，禁止直接 include Qt 头文件
                    └── Driver-Interface 层 ← 纯 C 头文件，虚基类（= 0），public virtual ~IXxxDriver() = default
                          └── Driver 层 ← 具体实现，封装 libssh2 / sqlite3 C API
```

**关键规则**：

- **只有 QML 层和 Model 层允许使用 Qt**，其余层全是纯 C/C++（仅 STL + 项目内头文件）
- Controller 持有 `std::unique_ptr<ISshDriver>` 等接口指针，通过依赖倒置不感知 Driver 实现
- Model 层通过 EventBus 与 Controller 通信，不直接调用 Controller 方法
- Driver-Interface 层是纯 C 头文件（非 C++），为最大化 ABI 兼容性
- `src/driver/mock/` 提供 Mock 驱动，供上层在无真实 SSH 环境时并行开发

## 项目文件结构

```
src/
  qml/          ← QML 页面/组件/主题/国际化
  model/        ← C++ Qt Model 类（ConnectionListModel, TableDataModel 等）
  eventbus/     ← EventBus 单例封装 + 事件名常量定义
  controller/   ← 业务逻辑（ConnectionController, SyncController 等）
  driver_interface/ ← 纯 C 虚基类头文件（i_ssh_driver.h, i_sftp_driver.h 等）
  driver/       ← 驱动实现（ssh/, sftp/, sqlite/, mock/）
  util/         ← 通用工具（Result<T,E>, 字符串/文件/crypto 工具）
libs/
  EventBus/     ← git submodule, 仅头文件事件总线库（MIT）
third_party/
  sqlite3/      ← sqlite3 amalgamation（sqlite3.c + sqlite3.h）
  nlohmann/     ← json.hpp 单头文件
  spdlog/       ← 日志库 header-only
```

## 编码规范要点

- **C++ 标准**：C++17；**C 标准**：C11（仅 Driver-Interface 和 sqlite3）
- **规范**：C++ Core Guidelines，clang-tidy 强制执行（配置见 `.clang-tidy`）
- **格式化**：clang-format，基于 Google Style，100 列，4 空格缩进（配置见 `.clang-format`）
- **命名**：类/接口 `PascalCase`（`ISshDriver`），函数/变量 `camelCase`，成员变量 `_` 后缀（`bus_`），文件名 `snake_case`
- **禁止裸 new/delete** → `std::make_unique` / `std::make_shared`
- **头文件**：`#pragma once`，include 顺序：对应 .h → 本项目 → 第三方 → 标准库
- **单文件上限**：函数 ≤ 40 行，QML 文件 ≤ 300 行

## Git 规范

- **分支模型**：Trunk-Based Development，`master` 直接 push 禁止，功能分支 ≤ 5 天
- **Commit**：Conventional Commits — `<type>(<scope>): <subject>`，中文 body
- **Scope**：`qml | model | eventbus | controller | driver-interface | driver | ssh | sftp | sqlite | sync | build | theme | i18n`
- **合并**：Squash and Merge，双平台（Windows + Linux）CI 全绿方可合并
- **Submodule**：`libs/EventBus` 锁定 release tag，升级需单独 commit 并说明理由

## 两种操作模式

| 模式        | 路径                                                             | 使用场景               |
| --------- | -------------------------------------------------------------- | ------------------ |
| 远程命令模式    | QML → Controller → ISshDriver → 远端 `sqlite3 -json` → 解析 stdout | 设备端 sqlite3 CLI 可用 |
| SFTP 同步模式 | SFTP 下载 → LocalSqliteDriver（sqlite3 C API）→ 冲突检测 → SFTP 上传     | 本地复杂操作             |

## 关键依赖

| 依赖                   | 版本      | 用途                   |
| -------------------- | ------- | -------------------- |
| Qt                   | 6.8 LTS | QML + Model 层        |
| libssh2              | 1.11+   | SSH + SFTP（BSD 协议）   |
| sqlite3 amalgamation | 3.45+   | 本地 SQLite 引擎         |
| nlohmann/json        | 3.11+   | 远程 sqlite3 JSON 输出解析 |
| spdlog               | 1.14+   | 日志                   |
| XQQYT/EventBus       | v1.1.2  | 事件总线（git submodule）  |

## 编译器

- **Windows**：MSVC 2022（`/W4 /WX`）
- **Linux**：GCC 13+（`-Wall -Wextra -Wpedantic -Werror`）

clang-tidy / clang-format 是 LLVM 工具链，不随 MSVC 安装。Windows 下通过 `pip install clang-tidy clang-format` 或下载 LLVM 发行版安装。

## 事件命名规范

采用 `<模块>:<动作>` 格式，字符串事件名：

```
ssh:connected / ssh:disconnected / ssh:error
query:request / query:result / query:error
sftp:progress / sftp:complete / sftp:error
sync:conflict / sync:complete
```

事件通过 `EventBus::publish()` 全异步分发，回调在线程池中执行。Model 层在回调中通过 Qt 信号槽将结果转到主线程更新 UI。
