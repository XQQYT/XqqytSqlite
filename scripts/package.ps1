# XqqytSqlite Windows 打包脚本
# 自动完成: MSVC 环境加载 → vcpkg 依赖安装 → CMake 构建 → windeployqt → CPack
#
# 前置条件:
#   1. MSVC Build Tools 2022: C:\BuildTools
#   2. Qt 6.8: E:\Qt6.8
#   3. vcpkg: E:\vcpkg (环境变量 VCPKG_ROOT)
#   4. NSIS 3.10: E:\NSIS
#
# 用法:
#   powershell -File .\scripts\package.ps1
#   powershell -File .\scripts\package.ps1 -Portable
#   powershell -File .\scripts\package.ps1 -Installer
#   powershell -File .\scripts\package.ps1 -Clean

param(
    [switch]$Portable,
    [switch]$Installer,
    [switch]$Clean
)

$ErrorActionPreference = "Continue"
$QtPath = "E:\Qt6.8"
$QtVersion = "6.8.3"
$MsysPath = "C:\BuildTools"
$VcpkgRoot = "E:\vcpkg"
$NsisPath = "E:\NSIS"
$ProjectRoot = $PSScriptRoot + "\.."
$BuildDir = "$ProjectRoot\build\win-msvc-release"
$PackageDir = "$ProjectRoot\package"
$AppName = "XqqytSqlite"
$Version = "0.1.0"

# 如果未指定标志，默认打包两种
if (-not $Portable -and -not $Installer) {
    $Portable = $true
    $Installer = $true
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " XqqytSqlite Windows Packaging" -ForegroundColor Cyan
Write-Host " Version: $Version" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# ===== Step 1: 清理 =====
if ($Clean) {
    Write-Host "`n[1/6] Cleaning..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
    if (Test-Path $PackageDir) { Remove-Item -Recurse -Force $PackageDir }
}

# ===== Step 2: 安装 vcpkg 依赖 =====
Write-Host "`n[2/6] Installing vcpkg dependencies..." -ForegroundColor Yellow
$env:VCPKG_ROOT = $VcpkgRoot
Push-Location $ProjectRoot
& "$VcpkgRoot\vcpkg.exe" install --triplet x64-windows
if ($LASTEXITCODE -ne 0) {
    Write-Warning "vcpkg install had issues (may be already installed)"
}
Pop-Location

# ===== Step 3: CMake 配置 =====
Write-Host "`n[3/6] Configuring CMake..." -ForegroundColor Yellow
$env:PATH = "$QtPath\Tools\CMake_64\bin;$env:PATH"
$env:PATH = "$QtPath\$QtVersion\msvc2022_64\bin;$env:PATH"
$env:Qt6_DIR = "$QtPath\$QtVersion\msvc2022_64"

Push-Location $ProjectRoot
cmake --preset win-msvc-release
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

# ===== Step 4: 编译 =====
Write-Host "`n[4/6] Building Release..." -ForegroundColor Yellow
cmake --build --preset win-msvc-release --config Release
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

# ===== Step 5: windeployqt =====
Write-Host "`n[5/6] Collecting Qt runtime..." -ForegroundColor Yellow
$windeployqt = "$QtPath\$QtVersion\msvc2022_64\bin\windeployqt.exe"
$exePath = "$BuildDir\Release\$AppName.exe"

if (Test-Path $windeployqt) {
    & $windeployqt $exePath --qmldir "$ProjectRoot\src\qml" --no-translations --no-opengl-sw
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "windeployqt reported warnings"
    }

    # 复制 vcpkg 运行时 DLL
    $vcpkgBin = "$VcpkgRoot\installed\x64-windows\bin"
    if (Test-Path $vcpkgBin) {
        @("libssh2.dll", "libcrypto-3-x64.dll", "libssl-3-x64.dll", "zlib1.dll") | ForEach-Object {
            $src = Join-Path $vcpkgBin $_
            if (Test-Path $src) {
                Copy-Item $src "$BuildDir\Release\" -Force
                Write-Host "  Copied $_"
            }
        }
    }
} else {
    Write-Warning "windeployqt not found. Package may be missing Qt DLLs."
}

# ===== Step 6: CPack 打包 =====
Write-Host "`n[6/6] Creating packages..." -ForegroundColor Yellow
New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

$env:PATH = "$NsisPath;$env:PATH"
Push-Location $BuildDir

if ($Portable) {
    Write-Host "  Building portable ZIP..."
    cpack -G ZIP -C Release
    if ($LASTEXITCODE -eq 0) {
        $zipFile = Get-ChildItem "*.zip" | Sort-Object LastWriteTime -Desc | Select-Object -First 1
        if ($zipFile) {
            Move-Item $zipFile.FullName $PackageDir -Force
            Write-Host "  ✅ ZIP: $PackageDir\$($zipFile.Name)" -ForegroundColor Green
        }
    }
}

if ($Installer) {
    Write-Host "  Building NSIS installer..."
    cpack -G NSIS -C Release
    if ($LASTEXITCODE -eq 0) {
        $exeFile = Get-ChildItem "*.exe" | Sort-Object LastWriteTime -Desc | Select-Object -First 1
        if ($exeFile) {
            Move-Item $exeFile.FullName $PackageDir -Force
            Write-Host "  ✅ NSIS: $PackageDir\$($exeFile.Name)" -ForegroundColor Green
        }
    }
}

Pop-Location
Pop-Location

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " Packaging Complete!" -ForegroundColor Green
Write-Host " Output: $PackageDir" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

Get-ChildItem $PackageDir | ForEach-Object {
    $size = "{0:N1} MB" -f ($_.Length / 1MB)
    Write-Host "  $($_.Name)  ($size)" -ForegroundColor White
}
