# LVGL UI Generator — Claude Code Skill

[![Version](https://img.shields.io/badge/version-2.0-blue)](https://github.com/ajhdkjsahd/lvgl-ui-generator)
[![LVGL](https://img.shields.io/badge/LVGL-v9-2196F3)](https://lvgl.io/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

**LVGL v9 高级 UI 设计 & 代码生成 Skill。设计师思维，不是代码生成器。**

> 先用 HTML/CSS 做可视化预览 — 你看到效果，确认满意，再翻译为 LVGL C 代码。设计迭代 5 秒一轮，不需要编译烧录。

> 🚀 **超级模式**：与 Anthropic 官方的 `frontend-design` 插件联动使用 — 前者当**设计总监**（定制调色板/字体/签名元素），后者当**实现工程师**（嵌入式约束/C 代码/编译验证）。详见 [`references/frontend-design-integration.md`](references/frontend-design-integration.md)。

---

## 为什么需要这个 Skill

直接用 AI 生成 LVGL C 代码有致命问题：**你看不到画面。**

AI 说"深空渐变背景 + 锥形渐变金属表圈 + 金色时针带 glow 光晕"——你脑补不出效果，只能编译烧录到真机才能看到，不满意就重来。一轮 5-10 分钟。

**这个 Skill 解决了这个问题：**
1. AI 生成 HTML 预览（CSS 模拟所有 LVGL 视觉效果）
2. 启动本地 HTTP 服务器 → 你在浏览器打开 → 直接看到效果
3. 不满意？告诉 AI 改 HTML → 刷新浏览器即可
4. 满意 → 一键翻译为 LVGL v9 C 代码

---

## 能设计什么

| 能力 | 覆盖 |
|------|------|
| **视觉设计** | 光影/渐变/质感/变换/绘制 5 大系统，30+ 特效配方 |
| **配色方案** | 8 种预设风格（深海/工业/家居/医疗/奢华/赛博朋克/自然/暗色） |
| **圆屏手表** | 表盘 6 层结构、指针系统、刻度环、星点动画、手势交互 |
| **毛玻璃** | `backdrop-filter: blur()` → `blur_backdrop` + `blur_radius` |
| **金属拉丝** | `conic-gradient()` → `lv_grad_conical_init` + `LV_GRAD_EXTEND_REFLECT` |
| **霓虹发光** | `box-shadow: 0 0 30px accent` → `shadow_color=accent` + `shadow_width=30` |
| **3D 按钮** | 渐变 + 阴影 + translate 下移 → 按下时"沉下去" |
| **深空暗角** | `radial-gradient()` → `lv_grad_radial_init` + vignette |
| **脉冲呼吸** | `@keyframes` → `lv_anim_t` + `LV_ANIM_REPEAT_INFINITE` |
| **星点闪烁** | JS 随机生成 → `lv_timer` 周期更新 opa |
| **CSS → LVGL** | 完整翻译表：布局 18 项 + 视觉 22 项 + 颜色 + 字体 |

---

## 快速开始

```bash
# 1. 安装 Skill
git clone https://github.com/ajhdkjsahd/lvgl-ui-generator.git \
  ~/.agents/skills/lvgl-ui-generator

# 2. 在 Claude Code 中说：
#    "设计一个圆屏手表的深空极光表盘"
#    → Skill 自动触发 → 头脑风暴 → 生成 HTML 预览 → 启动本地 HTTP → 你在浏览器查看 → 你确认 → C 代码生成 → 编译验证
```

---

## 工作流

```
你描述需求
  ↓
lvgl-ui-generator 收集项目上下文（分辨率/字体/页面数/回调）
  ↓
┌── frontend-design 是否可用？ ──────────────┐
│ YES ↓                      NO ↓            │
│ 设计总监：frontend-design   预设色板表      │
│   → 定制调色板 (4-6 hex)    → 选 1/8        │
│   → 字体角色 (display/body)                 │
│   → 布局概念 + ASCII wireframe              │
│   → 签名元素（让人记住的特征）               │
└────────────────────────────────────────────┘
  ↓
lvgl-ui-generator 翻译设计为 HTML 预览
  ↓
启动本地 HTTP → 你在浏览器查看
  ↓
你反馈 → 改 HTML → 刷新（迭代到你满意）
  ↓
你确认："就这样！"
  ↓
AI 按 CSS → LVGL 翻译表生成 C 代码
  ↓
cmake --build 编译验证
  ↓
编译通过 → 部署到板子
```

---

## 文件结构

```
lvgl-ui-generator/
├── SKILL.md                                ← 主文档（强制关卡 + 场景匹配 + 完整工作流）
├── README.md                               ← 本文件
└── references/
    ├── design-tokens.md                    ← 🆕 Design Token 系统（间距/圆角/阴影/8套配色 token）
    ├── scenario-quickstarts.md             ← 🆕 7种场景快速入门（布局策略+代码模板+陷阱）
    ├── web-preview-workflow.md             ← Web 预览：HTML模板(6种设备+7种组件) + CSS→LVGL翻译表
    ├── lvgl-v9-api-cheatsheet.md           ← 控件 API + 完整样式属性(60+) + 事件/动画
    ├── visual-effects-catalog.md           ← 30+ 特效配方(毛玻璃/金属/霓虹…)
    ├── design-showcase.md                  ← 4 个完整设计案例(深海/工业/家居/医疗)
    ├── coding-conventions.md               ← 代码模板 + 命名约定 + v8→v9 + 性能约束
    ├── theme-system.md                     ← 主题系统 + 深浅色切换 + 陷阱
    ├── scrollbar-guide.md                  ← 滚动条场景决策树 + NO_SCROLL 宏
    ├── font-pipeline.md                    ← 5 种字体方案 + FA6 合并字体 + lv_font_conv
    ├── flex-grid-guide.md                  ← Flex vs Grid 决策树 + 陷阱
    ├── interaction-patterns.md             ← snprintf + 事件回调 + Toast + ctx 管理
    ├── icon-display-guide.md               ← LV_SYMBOL 速查 + FA6 图标方案
    ├── screen-navigation.md                ← 屏幕栈 push/pop + TabView + 生命周期
    ├── multi-dpi-guide.md                  ← lv_dpx/lv_pct/Grid FR/分辨率断点
    ├── animation-guide.md                  ← 6 种动画模式 + Timeline + 缓动选择
    ├── preview-workflow.md                 ← SDL PC 预览：编译验证真机效果
    ├── chart-guide.md                      ← lv_chart 数据图表模板(多图/点击查看值)
    ├── component-reuse.md                  ← L1/L2/L3 组件复用三层模式
    ├── common-errors.md                    ← 40+ 实战错误速查(现象→根因→修复)
    └── examples/
        ├── smart-home/                     ← 真机验证示例：传感器卡片网格 + 场景页
        └── motor-control/                  ← 真机验证示例：Arc 控件 + 模式芯片
```

---

## 设计理念

### 三明治结构

```
skill.md          (~180 行)  每次触发加载
references/       (~4000 行) 按需加载
```

SKILL.md 只包含**触发条件 + 核心原则 + 设计工具箱 + 关键决策 + 自检清单**。详细的 API、代码模板、错误速查放在 references/，由模型按需读取。每次触发仅消耗 ~5K tokens。

### 设计先行

这个 Skill 的核心原则是 **"设计先行，代码随后"**。每设计一个页面至少使用 3 种以上不同的视觉手法（光影+渐变+质感）——如果只用 bg_color + border + radius，说明设计不够。

### 每页有性格

不同页面有不同视觉锚点和手法组合。不套统一模板，每页独立设计。

---

## 致谢

- [LVGL](https://lvgl.io/) — 嵌入式 GUI 库
- 社区标杆项目：ZSWatch、OV-Watch、HoloCubic、X-Knob 等
- 实战经验来自 GEC6818 / S5P6818 ARM Linux 平台验证
