# Frontend-design + lvgl-ui-generator 集成

## 为什么需要二者联动

| 单独用 lvgl-ui-generator | 联动 frontend-design |
|--------------------------|---------------------|
| 从 8 套 preset 色板选 | 根据**主题内容**量身定制调色板 |
| "Industrial 风格" 是固定模板 | "精密仪器仪表盘" 有独特视觉身份 |
| 排版：FreeType 加载一个 TTF | 配对 display face + body face，字重/字距有意识选择 |
| 没有 signature 概念 | 必须有一个**让人记住的元素** |
| 可能沦为 AI 模板 | 自我批判后迭代到有辨识度 |

## 集成流程

```
┌─────────────────────────────────────────────────────────────┐
│ 用户: "做一个车床数控面板，车间用，耐脏，粗犷"                  │
├───────────────────────┬─────────────────────────────────────┤
│ lvgl-ui-generator     │ frontend-design                      │
│ (Phase A: 项目上下文) │ (Phase B: 视觉设计)                  │
├───────────────────────┼─────────────────────────────────────┤
│                       │                                     │
│ 问: 分辨率？          │ 设计 brief:                          │
│ → 1024×600           │   "车床CNC操作面板。车间环境，       │
│                       │   油污灰尘，需要耐脏、字体粗大、     │
│ 问: 字体？            │   按钮够大方便戴手套操作。"          │
│ → FreeType + 中文     │                                     │
│                       │ → 定制调色板:                        │
│ 问: 多少个页面？      │   bg:     #1A1C18 (深军绿灰)        │
│ → 单页，3 区          │   card:   #242820                    │
│                       │   accent: #E67E22 (暖橙，能透过油污) │
│ 问: 回调？            │   danger: #D35400                    │
│ → 急停按钮 + 参数 cb  │   text:   #E8E3D9 (暖白)            │
│                       │   text2:  #9A9488                    │
│                       │                                     │
│                       │ → 排版:                              │
│                       │   display: 粗体无衬线（28px，间距宽）│
│                       │   body:   等宽（16px，数据对齐）      │
│                       │                                     │
│                       │ → 布局概念:                          │
│                       │   ┌───────────────────────┐          │
│                       │   │  RPM   TEMP   STATUS  │          │
│                       │   │  1200  68°C   RUNNING │          │
│                       │   │───────────────────────│          │
│                       │   │                       │          │
│                       │   │  [大型 G-code 显示区]  │          │
│                       │   │                       │          │
│                       │   │───────────────────────│          │
│                       │   │  [急停]   [启动]  [归零]│          │
│                       │   └───────────────────────┘          │
│                       │                                     │
│                       │ → 签名元素:                          │
│                       │   急停按钮 — 直径80px物理蘑菇头感，    │
│                       │   红色脉冲发光，按下时整个面板变暗    │
│                       │   直到释放。                          │
├───────────────────────┴─────────────────────────────────────┤
│ lvgl-ui-generator (翻译)                                    │
│                                                             │
│ #define COL_BG       lv_color_hex(0x1A1C18)                 │
│ #define COL_ACCENT   lv_color_hex(0xE67E22)                 │
│ ...                                                         │
│                                                             │
│ lv_freetype_font_create(".../fonts/HeavySans-Bold.ttf", ...) │
│                                                             │
│ // 急停按钮: shadow_width=30 + shadow_opa=153 + 脉冲动画      │
│                                                             │
│ HTML preview → user confirms → C code → cmake --build       │
└─────────────────────────────────────────────────────────────┘
```

## 手把手步骤

### Step 1: lvgl-ui-generator 收集 Phase A 上下文

```markdown
AskUserQuestion:
  1. 分辨率？ (800×480 / 1024×600 / 480×272 / 自填)
  2. 字体策略？ (FreeType + 中文 / 内置英文 / lv_font_conv + FA6)
  3. 页面数量？ (单页 / 多页 TabView)
  4. 回调需求？ (无 / 有 → 描述)
```

### Step 2: 交给 frontend-design

**如果 frontend-design 可用**，用 Skill 工具调用它，把 Phase A 的上下文作为 brief 的一部分：

```
Skill("frontend-design", args: "设计一个LVGL嵌入式UI页面。
  场景: [用户描述]
  约束: 分辨率[W×H], 必须用[字体策略], [页面数]页, [回调需求]。
  请输出设计 token plan: 调色板(4-6个hex)、字体角色(display/body/utility)、
  布局概念(ASCII wireframe)、签名元素(一个让人记住的特征)。
  注意: 这是嵌入式设备，不是网页，没有浏览器渲染引擎。
  效果靠 LVGL 的 shadow/gradient/blur 等原生属性实现，CSS 无法直接使用。")
```

### Step 3: lvgl-ui-generator 翻译

拿到 frontend-design 的输出后，翻译动作：

| fd 输出 | lvgl 翻译 |
|---------|-----------|
| 调色板 hex | `#define COL_XXX lv_color_hex(0x...)` 放在 .c 顶部 |
| display face | `lv_freetype_font_create(path, ...)` 选对应字号 |
| body face | 同上，小字号 |
| 布局 ASCII wireframe | 拆解为 flex/grid 层级结构 |
| 签名元素 | 重点实现：特化 widget + glow/shadow/animation 组合 |

### Step 4: 正常流程继续

HTML preview → user confirms → C code → `cmake --build`

## 判断：frontend-design 是否可用

在 Phase A 结束后，用以下方式探测：

```
检查 Skill 工具中是否有 "frontend-design" 可用。
在 Claude Code 中，查看 skills 列表是否有 frontend-design。
如果用户安装了官方 frontend-design 插件，它会在会话中注册。
```

## 回退

如果 frontend-design 不可用，回退到 **lvgl-ui-generator 自带的 8 套预设色板**。
流程不变，但 Phase B 变成让用户从色板表里选一个。

## 注意事项

1. **不要重复设计** — frontend-design 已经定了色板，不要又问用户"用 Ocean 还是 Industrial"
2. **翻译不是改编** — 除非目标色深不支持（16位 RGB565 量化），否则忠实翻译 fd 的颜色
3. **LVGL 能力映射** — 如果 fd 的签名元素涉及 LVGL 无法实现的效果（如 CSS `mix-blend-mode`），要诚实告知并提替代方案
4. **嵌入式约束** — fd 不知道 LVGL 有 shadow/gradient/blur 等原生属性，这是你的专长，在翻译阶段发挥