# RemoteSQLite-Qt

**Remote SQLite management tool** — Built with Qt 6.8 LTS (QML + C++17).

Manage SQLite databases on remote devices via SSH, or download via SFTP for local editing with automatic conflict detection and sync.

![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue)
![C++](https://img.shields.io/badge/C++-17-blue)
![Qt](https://img.shields.io/badge/Qt-6.8_LTS-green)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

| Feature | Description |
|---------|-------------|
| 🔐 **SSH Connection** | Password / Private Key / SSH Agent authentication, ProxyJump, auto-reconnect |
| 📂 **SFTP Browser** | Browse remote directories, upload/download SQLite database files |
| 🗄️ **Database Browser** | Object tree (Tables/Views/Indexes/Triggers), table structure, paginated data |
| 📝 **SQL Editor** | Syntax highlighting, auto-complete, history, snippets, multi-tab |
| 🔄 **SFTP Sync** | Download → Local edit via native sqlite3 API → Conflict detection → Upload |
| 🏗️ **Table Designer** | Visual column/PK/FK/Index definition with real-time DDL preview |
| 🎨 **Neumorphism UI** | Dark/Light theme, convex/concave shadows, smooth animations |
| 🌐 **i18n** | Chinese / English |

## Screenshot

```
┌──────────────────────────────────────────────────────────────┐
│  Menu: File | Connection | View | Tools | Help               │
├──────┬───────────────────────────────────────────────────────┤
│      │  Tab: Database 1 | SQL Query 1 | +                    │
│ Side ├───────────────────────────────────────────────────────┤
│ Nav  │                                                       │
│      │  Main Content                                         │
│ 📁   │  (Data Browser / SQL Editor / Table Designer / SFTP)  │
│ 🗄️   │                                                       │
│ 📝   ├───────────────────────────────────────────────────────┤
│ 🔧   │  Result Panel (Results / Messages / Log)              │
├──────┴───────────────────────────────────────────────────────┤
│  Status: Connected | Remote | Rows: 100 | Time: 12.3ms       │
└──────────────────────────────────────────────────────────────┘
```

## Build

### Prerequisites

| Dependency | Version | Notes |
|-----------|---------|-------|
| Qt | 6.8 LTS | Quick + QuickControls2 + Qml modules |
| CMake | 3.27+ | |
| MSVC (Win) / GCC (Linux) | MSVC 2022 / GCC 13+ | C++17 required |
| vcpkg | latest | C/C++ package manager |
| libssh2 | 1.11+ | via vcpkg |
| clang-tidy / clang-format | latest | LLVM tools (optional, for linting) |

### Quick Start

```bash
# Clone with submodules
git clone --recurse-submodules <repo-url>
cd RemoteSQLite-Qt

# Configure (Windows + MSVC)
cmake --preset win-msvc-release

# Configure (Linux + GCC)
cmake --preset linux-gcc-release

# Build
cmake --build --preset win-msvc-release --config Release

# Run tests
ctest --preset default

# Lint
clang-tidy -config-file=.clang-tidy src/**/*.cpp -- -std=c++17
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h
```

## Architecture

Strict 6-layer architecture with top-down dependency:

```
QML Layer (Qt Quick)
  └── Model Layer (C++ + Qt)
        └── EventBus Layer (pure C++17, XQQYT/EventBus)
              └── Controller Layer (pure C++17, business logic)
                    └── Driver-Interface Layer (pure C headers)
                          └── Driver Layer (libssh2 + sqlite3)
```

**Key rules:**
- Only QML and Model layers use Qt
- Controller → Driver-Interface ← Driver (dependency inversion)
- Model ↔ Controller communicate via EventBus (async, thread-safe)
- EventBridge converts worker-thread callbacks to Qt main-thread signals

## Project Structure

```
src/
├── qml/          QML pages, components, themes, i18n
├── model/        C++ Qt models (QAbstractListModel, etc.)
├── eventbus/     EventBus singleton + Qt thread-safety bridge
├── controller/   Business logic controllers
├── driver_interface/  Pure virtual interfaces (C headers)
├── driver/       LibSSH2 SSH/SFTP, sqlite3 local, mock drivers
└── util/         Result<T,E>, data export, SQL formatter, etc.
```

## Dependencies

| Library | Version | License | Usage |
|---------|---------|---------|-------|
| Qt | 6.8 LTS | LGPLv3 | UI + Model |
| libssh2 | 1.11+ | BSD | SSH + SFTP |
| sqlite3 amalgamation | 3.45+ | Public Domain | Local SQLite engine |
| nlohmann/json | 3.11+ | MIT | Remote sqlite3 JSON output |
| spdlog | 1.14+ | MIT | Logging |
| XQQYT/EventBus | v1.1.2 | MIT | Event bus (git submodule) |

## Packaging

### Windows

Windows 提供两种打包方式，产物输出到 `package/` 目录：

| 方式 | 命令 | 产物 | 说明 |
|------|------|------|------|
| **便携 ZIP** | `cmake --package --preset win-portable-zip` | `RemoteSQLite-Qt-0.1.0-win64.zip` | 解压即用，包含所有 Qt + libssh2 运行时 DLL |
| **NSIS 安装包** | `cmake --package --preset win-nsis-installer` | `RemoteSQLite-Qt-0.1.0-win64.exe` | 标准安装向导，开始菜单 + 桌面快捷方式 + 卸载 |
| **全部** | `cmake --package --preset win-all` | ZIP + EXE | 同时生成两种格式 |

一键打包脚本（自动编译 + windeployqt + CPack）：

```powershell
# 同时生成 ZIP 和 NSIS 安装包
.\scripts\package.ps1

# 仅便携 ZIP
.\scripts\package.ps1 -Portable

# 仅 NSIS 安装包
.\scripts\package.ps1 -Installer

# 清理重编
.\scripts\package.ps1 -Clean
```

**前置条件**：
- Qt 6.8 安装于 `E:\Qt6.8`（可通过脚本变量修改）
- NSIS 已安装（`choco install nsis` 或 https://nsis.sourceforge.io/）
- `windeployqt.exe` 可用（Qt 自带）
- `VCPKG_ROOT` 环境变量已设置

### Linux

```bash
cmake --package --preset linux-deb-tgz  # 生成 .deb 和 .tar.gz
```

## License

MIT License — see [LICENSE](LICENSE)
