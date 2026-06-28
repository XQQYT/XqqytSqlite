# XqqytSqlite 开发环境配置脚本
# 用法: . .\scripts\env_setup.ps1  (在 PowerShell 中运行)

$ErrorActionPreference = "Stop"

Write-Host "=== XqqytSqlite 环境配置 ===" -ForegroundColor Cyan

# 1. vcpkg
$env:VCPKG_ROOT = "E:\vcpkg"
Write-Host "[vcpkg] VCPKG_ROOT = $env:VCPKG_ROOT" -ForegroundColor Green

# 2. MSVC Build Tools
$vsPath = "C:\BuildTools"
$vcvarsPath = "$vsPath\VC\Auxiliary\Build\vcvarsall.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "[MSVC] Found: $vsPath" -ForegroundColor Green
    # 加载 MSVC 环境
    cmd /c "`"$vcvarsPath`" x64 > NUL 2>&1 && set" | ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            $key = $Matches[1]
            $value = $Matches[2]
            if ($key -eq "PATH") {
                $env:PATH = $value + ";" + $env:PATH
            } else {
                Set-Item -Path "env:$key" -Value $value
            }
        }
    }
    Write-Host "[MSVC] Environment loaded" -ForegroundColor Green
} else {
    Write-Host "[MSVC] NOT FOUND at $vsPath" -ForegroundColor Yellow
    Write-Host "  请手动安装 Visual Studio Build Tools 2022" -ForegroundColor Yellow
    Write-Host "  下载: https://aka.ms/vs/17/release/vs_BuildTools.exe" -ForegroundColor Yellow
    Write-Host "  安装工作负荷: '使用 C++ 的桌面开发'" -ForegroundColor Yellow
}

# 3. Qt 6.8
$env:Qt6_DIR = "E:\Qt6.8\6.8.3\msvc2022_64"
Write-Host "[Qt] Qt6_DIR = $env:Qt6_DIR" -ForegroundColor Green

# 4. CMake (Qt 自带)
$cmakePath = "E:\Qt6.8\Tools\CMake_64\bin"
$env:PATH = "$cmakePath;$env:PATH"
Write-Host "[CMake] $cmakePath" -ForegroundColor Green

# 5. NSIS
$nsisPath = "E:\NSIS"
$env:PATH = "$nsisPath;$env:PATH"
Write-Host "[NSIS] makensis available" -ForegroundColor Green

# 6. Qt bin
$qtBinPath = "$env:Qt6_DIR\bin"
$env:PATH = "$qtBinPath;$env:PATH"
Write-Host "[Qt] bin added to PATH" -ForegroundColor Green

Write-Host ""
Write-Host "=== 环境就绪，开始打包 ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "  .\scripts\package.ps1            # 完整打包" -ForegroundColor White
Write-Host "  .\scripts\package.ps1 -Portable  # 仅便携 ZIP" -ForegroundColor White
Write-Host "  .\scripts\package.ps1 -Installer # 仅 NSIS 安装包" -ForegroundColor White
