# UI 预览工作流

Skill 内置最小 SDL 预览工程，用户无需手动配置 lv_port_pc_vscode。

## 快速开始

```bash
# 1. 克隆 skill（含预览工程）
git clone https://github.com/ajhdkjsahd/lvgl-ui-generator.git
cd lvgl-ui-generator

# 2. 写你的 UI 代码到项目 src/ui/pages/

# 3. 一键配置环境 (仅首次)
#    Windows: 右键 preview/setup.ps1 → "使用 PowerShell 运行"
#    Linux:   bash preview/setup.sh

# 4. 编译预览
cd preview
cmake -B build -G Ninja
cmake --build build

# 5. 运行 — 弹窗显示你的 UI
./build/main
```

## setup 脚本做了什么

| 步骤 | 说明 |
|------|------|
| 检测 MSYS2 | 自动查找 D:\msys64 或 C:\msys64 |
| 安装工具链 | `pacman -S gcc cmake ninja SDL2`（已有则跳过） |
| 克隆 LVGL | `git clone --depth 1` 到 `preview/lvgl/` |
| 复制 lv_conf.h | 从模板创建默认配置 |
| 编译 | cmake + ninja |

**仅首次需要**，后续只执行步骤 4-5。

## 预览工程结构

```
preview/
├── setup.ps1 / setup.sh   ← 一键环境配置
├── CMakeLists.txt          ← 最小构建文件
├── main.c                  ← 模板入口（改 #include 加载你的页面）
├── lv_conf.h               ← 自动生成
├── lvgl/                   ← setup 脚本克隆
└── build/                  ← 编译产物
```

## 针对不同平台的开发流程

```
PC 预览 (SDL)           ARM Linux              ESP32
─────────────────      ──────────────         ──────────
写 UI 代码             同一份 UI 代码          同一份 UI 代码
 ↓                     ↓                      ↓
preview/build/main     cmake 交叉编译          idf.py build
 ↓                     ↓                      ↓
秒级反馈 ✅            scp + ssh 部署         烧录
                       (分钟级)               (分钟级)
```

**UI 代码 100% 可移植** — 页面文件和字体文件不依赖任何平台。

## 接入已有项目

如果你的项目已经有 LVGL 环境（如 lv_port_pc_vscode），只需把 `preview/` 目录拷过去，修改 `CMakeLists.txt` 中的 `USER_ROOT` 路径即可。

## 限制

- 不支持硬件相关功能（GPIO、I2C 等）— 仅预览 UI 显示和交互
- SDL 模拟的触摸精度与真实触摸屏有差异
- 字体需用 lv_font_conv 预生成（位图），Tiny TTF 也可用
