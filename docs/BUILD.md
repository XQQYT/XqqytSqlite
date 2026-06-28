# XqqytSqlite 构建与打包指南

## 环境要求

| 组件                   | 版本        | 路径                        | 状态       |
| -------------------- | --------- | ------------------------- | -------- |
| Qt                   | 6.8.3 LTS | `E:\Qt6.8`                | ✅ 已安装    |
| CMake                | 3.30.5    | `E:\Qt6.8\Tools\CMake_64` | ✅ Qt 自带  |
| vcpkg                | latest    | `E:\vcpkg`                | ✅ 已安装    |
| NSIS                 | 3.10      | `E:\NSIS`                 | ✅ 已安装    |
| **MSVC Build Tools** | **2022**  | **需手动安装**                 | ❌ **缺失** |

## 第一步：安装 MSVC Build Tools 2022

> 由于命令行静默安装在当前环境遇到问题，请手动完成此步骤。

1. 下载 [Visual Studio Build Tools 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe)
   - 或使用已下载的文件：`E:\vs_BuildTools.exe`
2. 双击运行，选择 **"使用 C++ 的桌面开发"** 工作负荷
3. 在 **"单个组件"** 标签页中确保勾选：
   - `MSVC v143 - VS 2022 C++ x64/x86 生成工具`
   - `Windows 11 SDK (10.0.22621.0)`
   - `C++ CMake tools for Windows`
4. 安装位置：`E:\VS2022`（或默认 `C:\Program Files\...`）

## 第二步：配置环境并安装依赖

打开 **PowerShell**，运行：

```powershell
# 加载环境
. .\scripts\env_setup.ps1

# 安装 vcpkg 依赖
vcpkg install --triplet x64-windows libssh2 openssl zlib
```

## 第三步：构建

```powershell
# CMake 配置
cmake --preset win-msvc-release

# 编译
cmake --build --preset win-msvc-release --config Release

# 测试
ctest --preset default
```

> 如果 MSVC 安装在非默认位置，配置前需先加载 MSVC 环境：
> 
> ```cmd
> call "E:\VS2022\VC\Auxiliary\Build\vcvarsall.bat" x64
> ```

## 第四步：打包

```powershell
# 完整打包（ZIP + NSIS 安装包，自动完成 编译→windeployqt→CPack）
.\scripts\package.ps1

# 仅便携 ZIP
.\scripts\package.ps1 -Portable

# 仅 NSIS 安装包  
.\scripts\package.ps1 -Installer
```

打包产物输出到 `package\` 目录：

- `XqqytSqlite-0.1.0-win64.zip` — 便携版，解压即用
- `XqqytSqlite-0.1.0-win64.exe` — NSIS 安装包

## 第五步：运行

```powershell
# 便携版
.\build\win-msvc-release\Release\XqqytSqlite.exe

# 或安装版
"C:\Program Files\RemoXqqytSqlite\RemoteSQXqqytSqlite
```

## Linux 构建

```bash
# 安装依赖
sudo apt install build-essential cmake ninja-build gcc g++ libssh2-1-dev

# 配置
cmake --preset linux-gcc-release

# 编译
cmake --build --preset linux-gcc-release

# 打包 (DEB + TGZ)
cmake --package --preset linux-deb-tgz
```
