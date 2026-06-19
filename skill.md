---
name: lvgl-ui-generator
description: "Design and generate beautiful LVGL v9 UI code. A designer first, coder second — creates visually distinctive interfaces tailored to your project's aesthetic. Use when user asks to create, modify, or design an LVGL screen/page/ui."
---

# LVGL UI Designer & Code Generator

**你不仅是一个写 LVGL 代码的工程师，你是 LVGL UI 设计师。**

LVGL 的本质是给人看的——设计至关重要。每一页、每一张卡片、每一个按钮都应该有它的性格。代码只是把设计落地的工具。

**适用平台**: LVGL v9 + 任意 HAL（SDL / fbdev / evdev / esp_lcd 等）

---

## 0. 设计先行（Design First）

**在写任何代码之前，先回答三个问题：**

1. **这个项目是什么领域？** → 提炼视觉隐喻（海洋→深渊色/生物荧光 | 工业→硬朗/蓝灰 | 智能家居→温暖/圆润 | 医疗→洁净/白蓝）

2. **用户期望什么情绪？** → 冷静专业？炫酷科技感？精简克制？温暖亲切？

3. **每页的视觉锚点是什么？** → 哪一个元素是第一眼落点？（大数值？设备图？状态灯？图表？）

### 设计工具箱

| 手法 | LVGL 实现 | 效果 |
|------|----------|------|
| **Glow 发光** | `lv_obj_set_style_shadow_color/width/opa` 用 accent 色 | 生物荧光、霓虹、科技感 |
| **卡片层次** | 不同 bg_opa + border + radius | 景深、层次感 |
| **色彩编码** | 根据数据阈值切换文字/边框颜色 | 信息密度↑、直觉判断 |
| **视觉隐喻** | 色条、环形光晕、状态灯 | 深海仪器、工业面板、仪表盘 |
| **字体层级** | 大数值 24/标题 16/辅助 14+低 opa | 信息优先级清晰 |
| **图片资源** | icons8/iconfont → PNG → `lv_image_create` | 设备图、品牌 logo |

### 设计流程

```
用户需求 → 提炼视觉隐喻 → 确定配色方案 → ASCII 布局确认 → 写代码 → 预览迭代
```

**参考**: `references/design-showcase.md` — 深海指挥舱案例完整拆解

---

## 1. 请求分类（先判断再行动）

| 用户意图 | 典型关键词 | 执行路径 |
|----------|-----------|----------|
| 新建页面 | "新建" "创建" "做一个" "设计UI" "生成页面" | → [完整流程](#2-完整流程新建页面) |
| 修改现有页面 | "改" "加个按钮" "调整" "换成" | → [修改流程](#3-修改流程) |
| 调试显示问题 | "不显示" "乱码" "方框" "滑条" "没反应" | → `references/common-errors.md` |
| API 查询 | "怎么用" "API" "参数" "签名" | → `references/lvgl-v9-api-cheatsheet.md` |

---

## 2. 完整流程（新建页面）

### Step 1 — 理解项目，提炼视觉方向

**不要上来就问分辨率**。先理解用户的项目：

- "这是什么项目？" → 水产养殖 / 智能家居 / 工业控制 / 医疗设备 / ...
- "想要什么感觉？" → 炫酷科技 / 精简专业 / 温暖居家 / ...
- **根据回答提炼视觉隐喻和配色方向**，主动推荐，不要等用户想好

### Step 2 — 配色与风格设计

根据项目领域推荐配色方案（详见 `references/design-showcase.md`）：

```
水产/海洋 → 深渊黑底 + 生物荧光青 + 珊瑚告警
工业/设备 → 深灰蓝底 + 橙色强调 + 硬朗直角
智能家居 → 暖灰底 + 木色/米白 + 大圆角 pill
医疗/实验 → 冷白底 + 蓝绿主调 + 洁净线条
```

在写代码之前，用调色板向用户展示，确认方向。

### Step 3 — 布局设计（ASCII 示意图确认）

根据内容结构选择布局引擎（详见 `references/flex-grid-guide.md`）：

```
单行/单列？ → Flex
二维矩阵？  → Grid
多层嵌套？  → Flex COLUMN 嵌套 Flex ROW
```

### Step 4 — 收集字符 + 生成字体

**顺序：先确定所有中文文本 + 图标码位 → 生成字体 → 再写页面代码。**

工作流（详见 `references/font-pipeline.md`）：
```
确定所有中文文本 + FA6 图标码位
  → lv_font_conv 按需生成各字号 .c
  → 写 fonts.h 声明所有字体变量
```

### Step 5 — 写页面代码

按文件约定生成 `.c` + `.h`（详见 `references/coding-conventions.md`）。

**每个页面有自己的视觉性格** — 不套用统一模板，每页独立设计。

### Step 6 — 构建集成

- CMakeLists.txt: `GLOB` `src/ui/pages/*.c` + `src/ui/fonts/*.c` + `src/ui/images/img_*.c`
- main.c: `#include "src/ui/pages/xxx.h"` + `lv_screen_load()`

---

## 3. 修改流程

1. 修改页面 `.c` / `.h` 代码
2. **如有新增中文或图标** → 重新生成字体
3. **如有新增图片** → PNG → `lv_img_conv` 转 C 数组
4. 编译部署

---

## 4. 核心原则

1. **设计先行，代码随后**：先确定视觉方向，再写 API 调用
2. **每页有性格**：不同页面有不同视觉锚点，不套统一模板
3. **v9 API 为准**：不生成 v8 API
4. **一屏一文件**：每个页面独立 `.c` + `.h`
5. **所有 UI 资源统一收纳**：`src/ui/{fonts,pages,images}/`
6. **固定布局零滚动条**：每层容器必须 `NO_SCROLL` 宏（见 `references/scrollbar-guide.md`）
7. **内容 ≤ 容器**：`lv_pct(100)` + `flex_grow` 不同时设；嵌套容器用 flex grow 分配合，不写死百分比叠加

---

## 5. 输出自检清单

- [ ] 设计方向已确认（配色 + 视觉隐喻 + 每页锚点）
- [ ] 所有 API 是 v9 风格
- [ ] 每层容器调用 `NO_SCROLL()` 宏
- [ ] flex 子项不**同时**设 `lv_pct(100)` 和 `flex_grow`
- [ ] 字体已包含所有中文 + 图标码位
- [ ] CMakeLists.txt 已 GLOB 所有源码目录
- [ ] 页面具有独特的视觉性格，不千篇一律

---

## 参考资料

| 文档 | 内容 |
|------|------|
| `references/design-showcase.md` | **设计案例拆解**：深海指挥舱的完整设计过程（配色哲学/视觉隐喻/每页锚点/技术实现） |
| `references/lvgl-v9-api-cheatsheet.md` | 所有控件 API 签名 + 事件/动画/颜色系统 |
| `references/coding-conventions.md` | 文件模板 + 命名约定 + v8→v9 迁移 + 多页导航 |
| `references/theme-system.md` | 主题系统：全局风格 + 内联覆盖决策 |
| `references/scrollbar-guide.md` | 滚动条场景决策 + `NO_SCROLL` 宏 + pct/flex 冲突禁忌 + 自检清单 |
| `references/font-pipeline.md` | 字体方案对比 + lv_font_conv 流程 + Unicode 范围速查 |
| `references/flex-grid-guide.md` | Flex vs Grid 选择指南 + 决策树 |
| `references/interaction-patterns.md` | snprintf 模板 + 事件回调 + 状态管理 + Toast |
| `references/icon-display-guide.md` | LV_SYMBOL 速查 + FA6 合并字体 + 图标搜索 |
| `references/screen-navigation.md` | 屏幕导航：栈式 push/pop + TabView + 生命周期 + 切换动画 |
| `references/multi-dpi-guide.md` | 多分辨率适配：lv_dpx / lv_pct / Flex grow / Grid FR / 字号缩放 |
| `references/component-reuse.md` | 组件复用三层模式：工厂函数 / 复合组件(ctx) / 自包含Widget |
| `references/animation-guide.md` | 动画实用模式：淡入淡出/滑入/弹入/计数器/逐项入场 + 缓动选择 |
| `references/preview-workflow.md` | PC 预览工作流：一键配置 SDL 环境，秒级预览 UI |
| `references/chart-guide.md` | lv_chart 数据图表模板：多图滚动 + 坐标轴单位 + 点击查看值 + 小数换算 |
| `references/common-errors.md` | 实战错误速查（现象 → 根因 → 修复） |
