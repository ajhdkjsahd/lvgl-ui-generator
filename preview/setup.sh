#!/bin/bash
# LVGL UI Preview — Linux/Mac 一键配置脚本
set -e

echo "========================================"
echo "  LVGL UI Preview 环境配置"
echo "========================================"

# 1. 检测包管理器并安装依赖
if command -v apt-get &>/dev/null; then
    echo "[*] 安装依赖 (apt)..."
    sudo apt-get install -y gcc cmake ninja-build libsdl2-dev
elif command -v brew &>/dev/null; then
    echo "[*] 安装依赖 (brew)..."
    brew install cmake ninja sdl2
elif command -v dnf &>/dev/null; then
    echo "[*] 安装依赖 (dnf)..."
    sudo dnf install -y gcc cmake ninja-build SDL2-devel
elif command -v pacman &>/dev/null; then
    echo "[*] 安装依赖 (pacman)..."
    sudo pacman -S --noconfirm gcc cmake ninja sdl2
else
    echo "[!] 无法识别的包管理器，请手动安装: gcc cmake ninja SDL2"
    exit 1
fi
echo "[OK] 依赖安装完成"

# 2. 克隆 LVGL
SKILL_DIR="$(cd "$(dirname "$0")/.." && pwd)"
LVGL_DIR="$SKILL_DIR/preview/lvgl"
if [ ! -f "$LVGL_DIR/lvgl.h" ]; then
    echo "[*] 克隆 LVGL..."
    git clone --depth 1 https://github.com/lvgl/lvgl.git "$LVGL_DIR"
    echo "[OK] LVGL 已下载"
else
    echo "[OK] LVGL 已存在"
fi

# 3. 复制 lv_conf.h
if [ ! -f "$SKILL_DIR/preview/lv_conf.h" ]; then
    cp "$LVGL_DIR/lv_conf_template.h" "$SKILL_DIR/preview/lv_conf.h"
    echo "[*] 已创建 lv_conf.h (请将 #if 0 改为 #if 1)"
fi

# 4. 构建
echo "[*] 编译预览项目..."
cd "$SKILL_DIR/preview"
rm -rf build
cmake -B build -G Ninja
cmake --build build -j$(nproc)
echo ""
echo "========================================"
echo "  环境就绪! 运行: ./preview/build/main"
echo "========================================"
