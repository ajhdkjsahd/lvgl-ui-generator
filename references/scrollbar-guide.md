# 滚动条场景决策指南

## 场景分级（先判断，再决定策略）

嵌入式设备的 LVGL 界面**默认不应出现**让用户拖拽滚动的滚动条，但有例外场景。

### 决策树

```
需要容纳的内容超过当前可见区域？
├── 内容量固定（如卡片、状态面板）
│   → 设计问题：调整尺寸/字号/截断使内容 ≤ 容器
│   → 策略：严禁滚动条
│
├── 内容量可变但可预知上限（如设置页 10 项）
│   → 设计问题：控制项目数 ≤ 可见区域
│   → 策略：严禁滚动条，用分页或 Tab 拆分
│
├── 内容量不可预知（如设备列表 50+、日志页）
│   → 合理滚动场景
│   → 策略：**仅最外层容器**允许垂直滚动，内部子容器全部禁滚
│
└── 弹窗/对话框内容多
    → 策略：msgbox 自身可滚，内部保持截断
```

### 场景策略对照表

| 场景 | 是否该滚 | 策略 |
|------|----------|------|
| 卡片内文字溢出 | ❌ | 加大卡片 / 截断文字 / 减小字号 |
| 仪表盘布局 | ❌ | 布局固定，各卡片尺寸预计算 |
| Tab 内容区（多行卡片） | ⚠️ 仅外层 | Tab 页本身可垂直滚，内层卡片/row 禁滚 |
| 设备列表（50+项） | ✅ | 最外层容器垂直滚 |
| 设置页（单项少） | ❌ | 一屏放完 |
| 设置页（20+项） | ✅ | 按组拆分用 Tab，或整页滚 |
| 日志/终端输出 | ✅ | 整页滚 |
| 消息弹窗长文本 | ✅ | msgbox 自带滚动 |

---

## 根因（为什么会出现滚动条）

只要满足以下任一条件，LVGL 就会**自动**给容器加滚动条：
1. 子元素的 `width` 或 `height` 超出容器尺寸
2. `lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP)` 且文字换行后总高 > 容器可用高度
3. 容器本身没有用 `lv_obj_set_size` 限定，且内容溢出

---

## 代码层面硬性要求

### 禁止滚动的控件

```c
/* 1. 容器本身：禁止滚动 */
lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

/* 2. 文字标签：固定宽度 + 截断模式 */
lv_obj_set_width(label, 200);                          /* 固定像素 */
lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);     /* 截断，禁止 WRAP */

/* 3. 长页面：最外层整体滚动，内部子容器禁滚 */
lv_obj_set_scroll_dir(parent, LV_DIR_VER);             /* 只允许垂直 */
```

---

## 工厂函数

在 `src/ui/pages/smart_home.h` 中声明，实现中已默认禁滚动：

```c
lv_obj_t * create_inline_row(lv_obj_t * parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, w, h);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);  /* 透明行不拦截点击 */
    return row;
}
```

---

## 标签字号速算

按 16px 字体（中英文等宽）：

| 容器宽度 | 最多汉字 | 最多英文 | 实际可写 |
|----------|---------|---------|---------|
| 100px | 6 个 | 12 字符 | 短标签（"客厅灯"） |
| 200px | 12 个 | 24 字符 | 设备名 + 短描述 |
| 350px | 22 个 | 44 字符 | 完整描述 |

若要展示更长内容：**拆成多行**（多个 label），不要用 `LONG_WRAP`。

---

### 铁律宏

```c
/* 固定布局页面的每一层容器都必须调用 */
#define NO_SCROLL(obj) \
    lv_obj_set_scrollbar_mode((obj), LV_SCROLLBAR_MODE_OFF); \
    lv_obj_clear_flag((obj), LV_OBJ_FLAG_SCROLLABLE)
```

### 百分比 + flex_grow 禁忌

```c
// ❌ 同时设 pct 和 flex_grow — 两者冲突，必出滚动条
lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
lv_obj_set_flex_grow(grid, 1);

// ✅ 只设 flex_grow，高度用 LV_SIZE_CONTENT
lv_obj_set_size(grid, lv_pct(100), LV_SIZE_CONTENT);
lv_obj_set_flex_grow(grid, 1);
```

### 嵌套容器高度叠加

```c
// ❌ bar(4px) + body(100%) = 104%，溢出！
lv_obj_set_height(bar, 4);
lv_obj_set_height(body, lv_pct(100));

// ✅ 父用 flex column: bar(4px 不 grow) + body(flex_grow=1)
lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
lv_obj_set_size(bar, lv_pct(100), lv_dpx(4));   /* 不 grow */
lv_obj_set_size(body, lv_pct(100), LV_SIZE_CONTENT);
lv_obj_set_flex_grow(body, 1);                   /* 撑满剩余 */
```

## 自检清单

- [ ] **所有**容器对象调用 `NO_SCROLL()` 宏（非仅卡片）
- [ ] flex 子项**不**同时设 `lv_pct(100)` 和 `flex_grow`
- [ ] 嵌套容器用 flex column/grow 分配合，不写死百分比叠加
- [ ] 所有 `lv_label_create` 后跟 `lv_obj_set_width` + `LV_LABEL_LONG_CLIP`
- [ ] 全局 grep `LV_LABEL_LONG_WRAP` 应**无匹配**（列表/日志等合理场景除外）
- [ ] 可滚动的长列表/日志容器：仅最外层有 `LV_OBJ_FLAG_SCROLLABLE`，子元素全部禁滚

---

## 常见错误

| 现象 | 原因 | 修复 |
|------|------|------|
| 标签下方出现滑条 | 文字宽度超过标签 width | 加大 width 或缩短文字 |
| 卡片下方出现滑条 | 卡片内多行 label 总高 > 卡片高 | 改用截断 / 减少内容 / 加大卡片 |
| Tab 内容有滑条 | 内容总高 > Tab 高度 | 整页允许滚（合理），但**子容器**不能滚 |
| 页面整体意外滚动 | 内容总高 > 480 - TabBar高度(≈48px) = 432px | 精确计算：pad + title + grid + gaps ≤ 432px |
| **固定布局无故出现滚动条** 🔥🔥 | `lv_pct(100)` + `flex_grow` 同时设导致尺寸冲突；或嵌套 `lv_pct(100)` 叠加溢出 (bar 4px + body 100% = 104%) | flex 子项只设 `flex_grow`，高度用 `LV_SIZE_CONTENT`；每层容器加 `NO_SCROLL` 宏 |
| 深层子容器溢出被截断而不是滚动 | 父容器设了 `SCROLLABLE` 但子容器没清该 flag，LVGL 选择滚动最近的 scrollable 祖先 | 所有容器显式设 `scrollbar_mode = OFF` + 清 `SCROLLABLE`，用一个宏统一处理 |
