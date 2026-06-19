# LVGL UI Generator — Claude Code Skill

LVGL v9 UI 代码生成 skill，覆盖**Web预览确认 → 视觉设计 → 布局 → C代码生成 → 构建集成**全流程。

**核心创新**: 先用 HTML/CSS 做可视化预览（用户看到效果），确认满意后再翻译为 LVGL C 代码。

## 适用平台

- LVGL v9 所有平台：嵌入式 Linux (fbdev/evdev)、ESP32、PC 模拟器、STM32 等
- ARM 交叉编译（已验证 GEC6818 / S5P6818）
- 任意分辨率

## 文件结构

```
lvgl-ui-generator/
├── SKILL.md                             ← 主文档（触发 → 分类 → 流程 → 自检）
├── README.md                            ← 本文件
└── references/
    ├── lvgl-v9-api-cheatsheet.md        ← 控件 API + 完整样式属性(60+) + 事件/动画
    ├── web-preview-workflow.md          ← 🆕 Web 预览：HTML模板(含圆屏手表) + CSS→LVGL翻译
    ├── round-display-guide.md           ← 🆕 圆屏手表专项：表盘层级/指针/刻度/手势/性能
    ├── visual-effects-catalog.md        ← 🆕 视觉特效目录：30+ 特效配方
    ├── design-showcase.md               ← 🆕 4 个设计案例（深海/工业/家居/医疗）
    ├── coding-conventions.md            ← 代码模板 + 命名约定 + v8→v9
    ├── theme-system.md                  ← 主题系统 + 深浅色陷阱
    ├── scrollbar-guide.md               ← 滚动条场景决策树 + 自检
    ├── font-pipeline.md                 ← 字体方案 + FA6 合并字体
    ├── flex-grid-guide.md               ← Flex vs Grid + 决策树 + 陷阱
    ├── interaction-patterns.md          ← 事件 / 工厂函数 / Toast
    ├── icon-display-guide.md            ← LV_SYMBOL 速查 + 图标方案
    ├── screen-navigation.md             ← 栈/Tab/生命周期/动画
    ├── multi-dpi-guide.md               ← lv_pct/lv_dpx/Grid FR/断点
    ├── animation-guide.md               ← 动画模式 + Timeline + 缓动选择
    ├── preview-workflow.md              ← PC 预览工作流
    ├── chart-guide.md                   ← lv_chart 数据图表模板
    └── common-errors.md                 ← 实战错误速查
```

## 设计理念

### 三明治结构

```
SKILL.md          (~180 行)  每次触发加载 — 程序性指令
references/       (~3000 行) 按需加载 — 参考数据
```

### Token 效率

| 指标 | 重构前 | 重构后 |
|------|--------|--------|
| SKILL.md 行数 | 565 | ~180 |
| 每次触发 Token | ~15K | ~5K |
| 参考文档命中 | 按需 ~2-3 个 | 按需 ~2-3 个 |
| 视觉设计能力 | 6 行设计工具箱 | 完整视觉武器库 + 30+ 特效配方 |

## 能解决什么问题

| 模块 | 覆盖 |
|------|------|
| **Web 预览** | 🆕 HTML/CSS 可视化预览 → 用户确认 → 翻译为 LVGL C 代码 |
| **视觉设计** | 🆕 光影+渐变+质感+变换+绘制 5 大系统、8 种配色方案、4 个完整案例 |
| **视觉特效** | 🆕 毛玻璃/金属拉丝/霓虹发光/3D 按钮/Vignette/极光渐变等 30+ 配方 |
| **布局** | Flex vs Grid 选择指南 + 决策树 + translate_radial 圆形布局 |
| **主题** | `lv_theme_default_init` 全局风格 + 深浅色切换 + 陷阱 |
| **滚动条** | 场景分级决策树 + 根因分析 + 工厂函数 + 自检 |
| **图标** | 62 LV_SYMBOL 速查 + FA6 合并字体（2000+ 图标可选） |
| **中文** | 5 种字体方案对比 + lv_font_conv + FA6 合并 |
| **导航** | 屏幕栈 push/pop + TabView + 生命周期 + 切换动画 |
| **动画** | 6 种动画模式 + Timeline 编排 + 完整缓动库 |
| **交互** | snprintf 动态文本 + 事件回调 + 工厂函数 + Toast |
| **错误** | 16 条实战错误（现象 → 根因 → 修复） |
| **图表** | lv_chart 多图滚动 + 坐标轴 + 渐变填充区域 |

## 安装

```bash
git clone https://github.com/ajhdkjsahd/lvgl-ui-generator.git \
  ~/.agents/skills/lvgl-ui-generator
```

## 参考

- [LVGL 官方文档](https://docs.lvgl.io/)
- LVGL 源码内置符号：`lvgl/include/lvgl/font/lv_symbol_def.h`
