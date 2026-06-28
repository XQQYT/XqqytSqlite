$ErrorActionPreference = "Continue"
$env:VCPKG_ROOT = "E:\vcpkg"
$env:HTTP_PROXY = "http://127.0.0.1:7890"
$env:HTTPS_PROXY = "http://127.0.0.1:7890"
$QtPath = "E:\Qt6.8\6.8.3\msvc2022_64"
Set-Location "G:\XqqytSqlite"

Write-Host "=== Configure ==="
Remove-Item -Recurse -Force build\win -ErrorAction SilentlyContinue
$cfgOut = cmd /c "cd /d G:\XqqytSqlite && E:\Qt6.8\Tools\CMake_64\bin\cmake.exe -B build\win -G `"Visual Studio 17 2022`" -A x64 -DCMAKE_PREFIX_PATH=`"$QtPath`" -DCMAKE_TOOLCHAIN_FILE=`"E:\vcpkg\scripts\buildsystems\vcpkg.cmake`" -DVCPKG_TARGET_TRIPLET=x64-windows 2>&1"
if ($LASTEXITCODE -ne 0) { Write-Host "CONFIGURE FAILED"; exit 1 }
Write-Host "Configure OK"

Write-Host "`n=== Build ==="
$buildOut = cmd /c "cd /d G:\XqqytSqlite && E:\Qt6.8\Tools\CMake_64\bin\cmake.exe --build build\win --config Release --target XqqytSqlite 2>&1"
$errs = ($buildOut | Select-String "error C|error LNK").Count
$buildOut | Select-String "error C|error LNK" | Select-Object -First 5
Write-Host "$errs errors"

$exe = "G:\XqqytSqlite\build\win\Release\XqqytSqlite.exe"
if (Test-Path $exe) {
    $item = Get-Item $exe
    Write-Host "SUCCESS! $exe ($([math]::Round($item.Length/1KB,0)) KB)"
} else {
    Write-Host "EXE NOT FOUND"
}
