# LVGL UI Preview — Windows 一键配置脚本
# 用法: 右键 → "使用 PowerShell 运行" 或在终端执行 .\setup.ps1

$ErrorActionPreference = "Stop"
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  LVGL UI Preview 环境配置" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 1. 检查 MSYS2
$MSYS2 = "D:\msys64"
if (-not (Test-Path "$MSYS2\usr\bin\bash.exe")) {
    $MSYS2 = "C:\msys64"
}
if (-not (Test-Path "$MSYS2\usr\bin\bash.exe")) {
    Write-Host "[ERROR] 未找到 MSYS2, 请先安装: https://www.msys2.org/" -ForegroundColor Red
    Write-Host "  安装后确保路径为 D:\msys64 或 C:\msys64" -ForegroundColor Yellow
    exit 1
}
Write-Host "[OK] 找到 MSYS2: $MSYS2" -ForegroundColor Green

# 2. 安装编译工具链 + SDL2
Write-Host "[*] 安装 MinGW 工具链 + SDL2 (需要网络, 约 5 分钟)..." -ForegroundColor Yellow
& "$MSYS2\usr\bin\bash.exe" -lc "pacman -S --noconfirm --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-SDL2" 2>&1 | Out-Null
Write-Host "[OK] 工具链安装完成" -ForegroundColor Green

# 3. 检查 LVGL 子模块
$SKILL_DIR = Split-Path -Parent $PSScriptRoot
$LVGL_DIR = "$PSScriptRoot\lvgl"
if (-not (Test-Path "$LVGL_DIR\lvgl.h")) {
    Write-Host "[*] 克隆 LVGL (约 30MB)..." -ForegroundColor Yellow
    git clone --depth 1 https://github.com/lvgl/lvgl.git "$LVGL_DIR" 2>&1 | Out-Null
    Write-Host "[OK] LVGL 已下载" -ForegroundColor Green
} else {
    Write-Host "[OK] LVGL 已存在" -ForegroundColor Green
}

# 4. 复制 lv_conf.h 模板 (如果不存在)
if (-not (Test-Path "$PSScriptRoot\lv_conf.h")) {
    Copy-Item "$LVGL_DIR\lv_conf_template.h" "$PSScriptRoot\lv_conf.h"
    Write-Host "[*] 已创建 lv_conf.h (使用默认配置)" -ForegroundColor Yellow
    Write-Host "  如需自定义字体/功能, 编辑此文件后将 #if 0 改为 #if 1" -ForegroundColor Yellow
}

# 5. 构建
Write-Host "[*] 编译预览项目..." -ForegroundColor Yellow
$env:Path = "$MSYS2\mingw64\bin;$MSYS2\usr\bin;$env:Path"
Push-Location $PSScriptRoot
if (Test-Path build) { Remove-Item -Recurse -Force build }
& "$MSYS2\mingw64\bin\cmake.exe" -B build -G Ninja 2>&1
if ($LASTEXITCODE -ne 0) { Write-Host "[FAIL] CMake 配置失败" -ForegroundColor Red; exit 1 }
& "$MSYS2\mingw64\bin\cmake.exe" --build build -j $env:NUMBER_OF_PROCESSORS 2>&1
if ($LASTEXITCODE -ne 0) { Write-Host "[FAIL] 编译失败" -ForegroundColor Red; exit 1 }
Pop-Location

Write-Host "========================================" -ForegroundColor Green
Write-Host "  环境就绪! 运行: .\preview\build\main.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
