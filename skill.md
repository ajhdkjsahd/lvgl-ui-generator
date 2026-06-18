---
name: lvgl-ui-generator
description: "Design and generate LVGL v9 UI code (any platform: embedded Linux, ESP32, PC simulator, etc.). Use when user asks to create, modify, or design an LVGL screen/page/ui."
---

# LVGL UI Code Generator

为所有 LVGL v9 平台生成 UI 代码（嵌入式 Linux / ESP32 / PC 模拟器等）。

**适用平台**: LVGL v9 + 任意 HAL（SDL / fbdev / evdev / esp_lcd 等）

---

## 1. 请求分类（先判断再行动）

| 用户意图 | 典型关键词 | 执行路径 |
|----------|-----------|----------|
| 新建页面 | "新建" "创建" "做一个" "设计UI" "生成页面" | → [完整流程](#2-完整流程新建页面) |
| 修改现有页面 | "改" "加个按钮" "调整" "换成" | → [修改流程](#3-修改流程) |
| 调试显示问题 | "不显示" "乱码" "方框" "滑条" "没反应" | → `references/common-errors.md` |
| API 查询 | "怎么用" "API" "参数" "签名" | → `references/lvgl-v9-api-cheatsheet.md` |
| 编译部署 | "编译" "部署" "跑起来" "cmake" | → [构建与部署](#5-构建与部署) |

---

## 2. 完整流程（新建页面）

### Step 1 — 需求收集

向用户确认（已有默认值的可跳过）：

| 项目 | 默认值 |
|------|--------|
| 屏幕分辨率 | 800×480 |
| 颜色深度 | 16 (RGB565) |
| 主题风格 | 浅色/深色 |
| 页面结构 | 单页 / 多页+导航 |
| 交互方式 | 触摸 |

### Step 2 — 布局设计（ASCII 示意图确认）

在写代码之前，先用 ASCII 示意图向用户展示布局。根据内容结构选择布局引擎（详见 `references/flex-grid-guide.md`）：

```
单行/单列？ → Flex
二维矩阵？  → Grid
多层嵌套？  → Flex COLUMN 嵌套 Flex ROW
```

### Step 3 — 确定主题与风格

**主题初始化应在 `main.c` 的 display 创建之后调用，不在页面 create 函数中。**

用 LVGL v9 主题系统建立全局风格基准，减少 60%+ 的样式代码（详见 `references/theme-system.md`）：

```c
// main.c 中 display 初始化之后
lv_theme_t * th = lv_theme_default_init(lv_display_get_default(),
    lv_palette_main(LV_PALETTE_BLUE),
    lv_palette_main(LV_PALETTE_GREY),
    true, &lv_font_app_16);  /* true=深色 false=浅色 */
```

页面代码中仅用内联样式做**差异化**覆盖（如标题用更大字号）。

### Step 4 — 收集字符 + 生成字体

**顺序：先确定所有中文文本 → 生成字体 → 再写页面代码。**

工作流（详见 `references/font-pipeline.md`）：
```
确定 ASCII 布局中的所有中文
  → Python 扫描收集汉字 → src/ui/fonts/chars.txt
  → lv_font_conv 按需生成各字号 .c → src/ui/fonts/app_16.c, app_24.c, ...
  → 写 src/ui/fonts/app_fonts.h 声明所有字体变量
```

### Step 5 — 写页面代码

按文件约定生成 `.c` + `.h`（详见 `references/coding-conventions.md`）：

```
src/ui/
├── fonts/
│   ├── chars.txt
│   ├── app_16.c
│   ├── app_24.c
│   └── app_fonts.h
└── pages/
    ├── smart_home.h       # 共享头文件 + create_inline_row 等工厂函数
    ├── smart_home.c
    ├── home_page.c
    └── device_page.c
```

布局与滚动：`references/flex-grid-guide.md` + `references/scrollbar-guide.md`
交互/事件：`references/interaction-patterns.md`
图标使用：`references/icon-display-guide.md`

### Step 6 — 构建集成

- CMakeLists.txt: `GLOB` `src/ui/pages/*.c` + `src/ui/fonts/*.c`
- main.c: `#include "src/ui/pages/xxx.h"` + `lv_screen_load()`

---

## 3. 修改流程

1. 修改页面 `.c` / `.h` 代码
2. **如有新增中文** → 重新运行 Python 扫描 + `lv_font_conv` 生成字体
3. 编译部署

---

## 4. 核心原则

1. **先理解后编码**：不急于写代码，先用 ASCII 示意图确认布局
2. **v9 API 为准**：不生成 v8 API（`lv_scr_act` → `lv_screen_active` 等）
3. **一屏一文件**：每个页面独立 `.c` + `.h`，放在 `src/ui/pages/`
4. **所有 UI 资源统一收纳**：`src/ui/{fonts,pages}/`，方便整体移植
5. **内容 ≤ 容器**：固定布局严禁溢出滚动条；长列表/日志等场景仅外层允许滚动（决策见 `references/scrollbar-guide.md`）
6. **图标与中文分离**：不同字体不能混在同一 label（见 `references/icon-display-guide.md`）

---

## 5. 构建与部署

```bash
rm -rf build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake
cmake --build build -j$(nproc)
scp build/bin/lvglsim root@<板子IP>:/root/
ssh root@<板子IP> ./lvglsim
```

---

## 6. 输出自检清单

- [ ] 所有 API 是 v9 风格
- [ ] 每个页面独立 `.c` + `.h`，头文件有 include guard
- [ ] 所有 UI 资源在 `src/ui/` 下
- [ ] 主题系统已初始化（`lv_theme_default_init`），避免大量手写样式
- [ ] 字体：Python 扫描 → chars.txt → lv_font_conv（`--no-compress` + `-r 0x20-0x7F`）→ app_fonts.h
- [ ] **禁止**使用 `lv_font_source_han_sans_sc_16_cjk`
- [ ] 固定布局的卡片/row 全部 `LV_SCROLLBAR_MODE_OFF` + 无 `LV_LABEL_LONG_WRAP`
- [ ] 图标与中文分属两个 label，各用各的字体
- [ ] CMakeLists.txt 已 `GLOB` `src/ui/pages/*.c` 和 `src/ui/fonts/*.c`
- [ ] main.c 调用了 `xxx_create()` + `lv_screen_load()`

---

## 参考资料

| 文档 | 内容 |
|------|------|
| `references/lvgl-v9-api-cheatsheet.md` | 所有控件 API 签名 + 事件/动画/颜色系统 |
| `references/coding-conventions.md` | 文件模板 + 命名约定 + v8→v9 迁移 + 多页导航 |
| `references/theme-system.md` | 主题系统：全局风格 + 内联覆盖决策 |
| `references/scrollbar-guide.md` | 滚动条场景决策树 + 工厂函数 + 自检清单 |
| `references/font-pipeline.md` | 字体方案对比 + lv_font_conv 流程 + WSL 1 workaround |
| `references/flex-grid-guide.md` | Flex vs Grid 选择指南 + 决策树 |
| `references/interaction-patterns.md` | snprintf 模板 + 事件回调 + 状态管理 + Toast |
| `references/icon-display-guide.md` | LV_SYMBOL 速查 + 中文分离规则 + Dropdown 修复 |
| `references/screen-navigation.md` | 屏幕导航：栈式 push/pop + TabView + 生命周期 + 切换动画 |
| `references/multi-dpi-guide.md` | 多分辨率适配：lv_dpx / lv_pct / Flex grow / Grid FR / 字号缩放 / 分辨率分支 |
| `references/component-reuse.md` | 组件复用三层模式：工厂函数 / 复合组件(ctx) / 自包含Widget(生命周期) |
| `references/animation-guide.md` | 动画实用模式：淡入淡出/滑入/弹入/计数器/逐项入场 + 缓动选择 |
| `references/preview-workflow.md` | PC 预览工作流：一键配置 SDL 环境，秒级预览 UI |
| `references/common-errors.md` | 16 条实战错误速查（现象 → 根因 → 修复） |
