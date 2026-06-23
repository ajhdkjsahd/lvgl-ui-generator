# 场景快速入门

> **读用户需求 → 匹配场景 → 按此文件的模板起手。不要从零推导布局。**
>
> ⚠️ **重要**：以下代码模板展示的是**布局骨架**。实际使用时必须补全：样式初始化、颜色值（用 `lv_color_hex(0x...)`）、字体设置、事件回调。数值是具体像素值，可直接使用。

---

## 场景匹配表

| 用户关键词 | → 场景 | 跳转到 |
|-----------|--------|--------|
| 仪表盘/总览/监控/数据看板/传感器/Dashboard | **仪表盘** | §1 |
| 设置/配置/选项/参数 | **设置页** | §2 |
| 列表/设备列表/日志/记录 | **列表页** | §3 |
| 手表/表盘/指针/圆屏/时钟 | **表盘** | §4 |
| 控制/开关/旋钮/遥控/操控 | **控制面板** | §5 |
| 图表/曲线/趋势/折线图/Chart | **数据图表** | §6 |
| AI/对话/聊天/智能助手/消息 | **对话页** | §7 |
| 天气/温度/湿度 | 参考仪表盘或表盘 | §1 或 §4 |

---

## §1 仪表盘 (Dashboard)

### 识别
用户说"做个仪表盘"、"传感器监控面板"、"设备状态看板"、"总览页"

### 布局策略
```
Grid 2×N 或 3×N（卡片网格）
├── 每张卡片 = 一个传感器/设备
│   ├── icon + 标题（顶部行）
│   ├── 大数值（居中，颜色编码）
│   └── 单位 + 状态（底部行）
└── 可选：顶部 summary 条（总览数值）
```

### 代码模板
```c
/* Grid 容器 — 必须设 grid_dsc_array */
lv_obj_t * grid = lv_obj_create(screen);
lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
lv_obj_set_style_pad_all(grid, 16, 0);
lv_obj_set_style_pad_row(grid, 12, 0);
lv_obj_set_style_pad_column(grid, 12, 0);

static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
NO_SCROLL(grid);

/* 每张卡片：用 L2 工厂函数 */
/* ⚠️ create_sensor_card 需要自己实现，返回包含 card + value_label 的结构体 */
/* 参考: references/examples/smart-home/home_page.c */
sensor_card_t cards[6];
for (int i = 0; i < 6; i++) {
    cards[i] = create_sensor_card(grid, icons[i], titles[i], units[i]);
    lv_obj_set_grid_cell(cards[i].card, LV_GRID_ALIGN_STRETCH, i % 2, 1,
                                          LV_GRID_ALIGN_STRETCH, i / 2, 1);
}
```

### 组件模式
- **L2 工厂函数** `create_sensor_card()` — 保存 `card` + `value_label` + `unit_label` 引用
- 每个传感器一种 accent 色条（卡片顶部或左侧边框着色）

### 视觉锚点
- 核心数据**大数值**（24-32px），颜色编码（正常=白，告警=红）
- 每张卡片**不同 accent 色条**作为视觉区分

### 常见陷阱
- ❌ 用 Flex column 嵌套代替 Grid → 布局不齐
- ❌ 数值和单位字号相同 → 无视觉层次
- ❌ 所有卡片同色 → 无法快速区分传感器
- ❌ 数值没有颜色编码 → 无法一眼判断状态
- ❌ **忘记在 grid 上设 pad_all/pad_row/pad_column → 卡片挤在一起**

### 参考示例
`references/examples/smart-home/home_page.c` — 传感器卡片网格


## §2 设置页 (Settings)

### 识别
用户说"做个设置页面"、"参数配置"、"选项页"、"系统设置"

### 布局策略
```
Flex COLUMN（外层可滚）
├── 分组标题 label
├── toggle_row × N（开关项）
├── 分组标题 label
├── menu_row × N（跳转子页项）
└── ...
```

### 代码模板
```c
/* 最外层可滚容器 */
lv_obj_t * page = lv_obj_create(parent);
lv_obj_set_size(page, lv_pct(100), lv_pct(100));
lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_scroll_dir(page, LV_DIR_VER);  // 仅外层可滚
lv_obj_set_style_pad_all(page, 16, 0);
lv_obj_set_style_pad_row(page, 4, 0);

/* 分组标题 */
lv_obj_t * section_title = lv_label_create(page);
lv_obj_set_style_text_font(section_title, &lv_font_app_16, 0);
lv_obj_set_style_text_color(section_title, lv_color_hex(0x888888), 0);
lv_obj_set_style_pad_hor(section_title, 16, 0);
lv_obj_set_style_pad_ver(section_title, 12, 0);
lv_label_set_text(section_title, "网络设置");

/* 开关行：L3 自包含组件 */
/* ⚠️ toggle_row_create 需要自己实现，参考 references/component-reuse.md L3 示例 */
toggle_row_t * wifi_row = toggle_row_create(page, "WiFi", false, on_wifi_changed, NULL);

/* 跳转行（点击进入子页） */
lv_obj_t * menu_row = lv_obj_create(page);
lv_obj_set_size(menu_row, lv_pct(100), 48);
lv_obj_set_flex_flow(menu_row, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(menu_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_style_pad_hor(menu_row, 16, 0);
NO_SCROLL(menu_row);
// 左侧: icon + text / 右侧: 箭头或当前值
```

### 组件模式
- **L3 自包含 Widget** `toggle_row_create()` — 开关行（有内部状态和回调）
- 简单跳转行用 L1 工厂函数

### 视觉锚点
- 清晰的分组标题作为视觉分隔
- 开关的 accent 色作为唯一彩色元素
- 整体简洁，信息密度低

### 常见陷阱
- ❌ 内容多时拒绝滚动 → 设置页是少数允许滚动的场景
- ❌ 开关行用 L1 手写 → 重复代码多，应升级到 L3
- ❌ 没有分组标题 → 一堆开关排列，用户找不到位置
- ❌ **内层 menu_row/toggle_row 忘记 NO_SCROLL → 每行都有滚动条**


## §3 列表页 (List)

### 识别
用户说"做个设备列表"、"日志页"、"记录列表"

### 布局策略
```
Flex COLUMN（外层可滚）
├── list_item × N
│   ├── 左侧：icon / 状态 LED
│   ├── 中间：标题 + 描述（column）
│   └── 右侧：状态文字 / 箭头
└── ...
```

### 代码模板
```c
lv_obj_t * list = lv_obj_create(parent);
lv_obj_set_size(list, lv_pct(100), lv_pct(100));
lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_scroll_dir(list, LV_DIR_VER);
lv_obj_set_style_pad_all(list, 16, 0);
lv_obj_set_style_pad_row(list, 8, 0);  // 项间距

for (int i = 0; i < count; i++) {
    lv_obj_t * item = lv_obj_create(list);
    lv_obj_set_size(item, lv_pct(100), 64);
    lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(item, 16, 0);
    lv_obj_set_style_pad_row(item, 8, 0);
    NO_SCROLL(item);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_CLICKABLE);  // 透明行不拦截点击

    // 左侧 icon
    lv_obj_t * icon = lv_label_create(item);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
    lv_label_set_text(icon, LV_SYMBOL_WIFI);

    // 中间信息列
    lv_obj_t * info_col = lv_obj_create(item);
    lv_obj_set_flex_flow(info_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(info_col, 1);
    lv_obj_set_style_bg_opa(info_col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(info_col, 0, 0);
    lv_obj_set_style_pad_all(info_col, 0, 0);
    NO_SCROLL(info_col);

    lv_obj_t * title = lv_label_create(info_col);
    lv_label_set_text(title, "客厅灯");

    lv_obj_t * desc = lv_label_create(info_col);
    lv_obj_set_style_text_color(desc, lv_color_hex(0x888888), 0);
    lv_label_set_text(desc, "已开启 · 亮度 80%");

    // 右侧箭头
    lv_obj_t * arrow = lv_label_create(item);
    lv_obj_set_style_text_font(arrow, &lv_font_montserrat_24, 0);
    lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
}
```

### 组件模式
- **L2 工厂函数** `create_list_item()` — 列表项
- 如果列表项有交互（点击/swipe），升级到 L3

### 视觉锚点
- 左侧状态 LED 或图标
- 标题醒目，描述文字淡化（`lv_color_hex(0x888888)`）
- 项之间有微妙的边框或底色区分

### 常见陷阱
- ❌ 内层 item 没关 SCROLLABLE → 每行都有滚动条
- ❌ 每一项硬编码 → 应用工厂函数
- ❌ 列表项高度不统一 → 60-72px 为佳
- ❌ **item 内部子容器忘记清 CLICKABLE → 点击被透明容器拦截**


## §4 表盘 (Watch Face)

### 识别
用户说"设计一个手表表盘"、"圆屏UI"、"指针式时钟"

### 布局策略
```
绝对定位（圆形容器内）
├── 背景：径向渐变 + 星点（DRAW_MAIN 或 Canvas）
├── 表圈：锥形渐变金属环
├── 刻度：60 个 tick（5 的倍数加粗）
├── 数字时间：居中大字
├── 日期/步数：辅助文字
├── 指针：时针(HOUR) + 分针(MINUTE) + 秒针(SECOND)
│   └── 旋转用 lv_image_set_rotation() + lv_anim
├── 中心帽：径向渐变小圆
└── 品牌文字：底部居中
```

### 代码模板
```c
/* 圆形容器 */
lv_obj_t * face = lv_obj_create(NULL);
lv_obj_set_size(face, 360, 360);
lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
lv_obj_set_style_clip_corner(face, true, 0);
NO_SCROLL(face);

/* 径向渐变背景 */
static const lv_color_t grad_colors[2] = {
    LV_COLOR_MAKE(0x1E, 0x2A, 0x4A),   // 中心稍亮
    LV_COLOR_MAKE(0x0B, 0x12, 0x2E),   // 边缘深色
};
static lv_grad_dsc_t grad;
lv_grad_init_stops(&grad, grad_colors, NULL, NULL, 2);
lv_grad_radial_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER,
                    LV_GRAD_RIGHT, LV_GRAD_BOTTOM, LV_GRAD_EXTEND_PAD);
lv_obj_set_style_bg_grad(face, &grad, 0);

/* 指针（用图片） */
lv_obj_t * hour_hand = lv_image_create(face);
lv_image_set_src(hour_hand, &img_hand_hour);
lv_image_set_pivot(hour_hand, 2, 64);   // pivot 在指针底部中心
lv_image_set_rotation(hour_hand, 0);     // 初始角度

/* 动画驱动指针旋转 */
lv_anim_t a;
lv_anim_init(&a);
lv_anim_set_var(&a, hour_hand);
lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_image_set_rotation);
lv_anim_set_values(&a, 0, 3600);  // 0° → 360° (×10)
lv_anim_set_duration(&a, 43200000);  // 12小时
lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
lv_anim_start(&a);
```

### 组件模式
- 刻度：DRAW_TASK_ADDED 事件自定义绘制，或预创建 60 个 label
- 星点：lv_timer 周期性更新 opa（不创建/删除对象）
- 指针：lv_anim 驱动 `lv_image_set_rotation`

### 视觉锚点
- 指针是核心视觉元素（用 glow 投影突出）
- 品牌名在底部提升高级感
- 星点闪烁增加动态细节

### 常见陷阱
- ❌ 同时动画超过 3 个 → MCU 掉帧
- ❌ 用 Tiny TTF 做秒针字体 → 首次渲染延迟
- ❌ 每帧重建星点对象 → 性能灾难

### 参考
`references/round-display-guide.md` — 完整表盘层级和性能约束


## §5 控制面板 (Control Panel)

### 识别
用户说"控制面板"、"遥控器界面"、"设备操控"、"旋钮"

### 布局策略
```
Grid 不等宽 或 Flex 混合
├── 大圆形控件（Arc/旋钮）→ Grid FR(2)
├── 开关/按钮群 → Grid FR(1) 或 Flex ROW
├── 模式芯片（chip toggle）→ Flex ROW_WRAP
└── 状态指示灯 → 小 LED 点
```

### 代码模板
```c
/* 大圆形 Arc 控件 */
lv_obj_t * arc = lv_arc_create(parent);
lv_obj_set_size(arc, 160, 160);
lv_obj_center(arc);
lv_arc_set_range(arc, 0, 100);
lv_arc_set_value(arc, 75);

/* 模式芯片（代替滑块） */
lv_obj_t * chip_row = lv_obj_create(parent);
lv_obj_set_size(chip_row, lv_pct(100), LV_SIZE_CONTENT);
lv_obj_set_flex_flow(chip_row, LV_FLEX_FLOW_ROW_WRAP);
lv_obj_set_flex_align(chip_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_style_pad_all(chip_row, 0, 0);
lv_obj_set_style_pad_row(chip_row, 8, 0);
lv_obj_set_style_pad_column(chip_row, 8, 0);
NO_SCROLL(chip_row);

lv_obj_t * chips[4];
const char * modes[] = {"自动", "制冷", "制热", "送风"};
for (int i = 0; i < 4; i++) {
    chips[i] = lv_obj_create(chip_row);
    lv_obj_set_size(chips[i], LV_SIZE_CONTENT, 40);
    lv_obj_set_style_pad_hor(chips[i], 20, 0);
    lv_obj_set_style_radius(chips[i], 20, 0);
    NO_SCROLL(chips[i]);

    lv_obj_t * label = lv_label_create(chips[i]);
    lv_label_set_text(label, modes[i]);
    lv_obj_center(label);

    // 选中态/未选中态在事件回调中切换
    lv_obj_add_event_cb(chips[i], on_chip_click, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
}

/* 默认选中第一个 */
lv_obj_set_style_bg_color(chips[0], lv_color_hex(0xFF9800), 0);
lv_obj_set_style_bg_opa(chips[0], LV_OPA_30, 0);
lv_obj_set_style_border_color(chips[0], lv_color_hex(0xFF9800), 0);
```

### 组件模式
- **Arc 控件**：温度/亮度/速度调节
- **模式芯片**：多选一（比 dropdown 更直观）
- **圆形按钮**：电源键（大号 + GLOW 发光）
- **Switch**：简单的开/关

### 视觉锚点
- 大圆形控件是视觉焦点（Arc 或圆按钮）
- 电源按钮用 accent + glow 发光突出
- 芯片选中态用 accent 色 + 高 opa

### 常见陷阱
- ❌ Arc 尺寸 < 120px → 手指无法操作
- ❌ 模式用 dropdown → 嵌入式上操作不便，用芯片代替
- ❌ 电源按钮尺寸太小 → 圆形按钮至少 64px
- ❌ **芯片 row 忘记 NO_SCROLL → chip_row 出现滚动条**

### 参考示例
`references/examples/motor-control/page_dc.c` — Arc 控件 + 模式芯片


## §6 数据图表 (Chart)

### 识别
用户说"曲线图"、"折线图"、"趋势图"、"数据可视化"

### 布局策略
```
Grid 1×N 或 2×N（多图）
├── 每张图 = chart 容器
│   ├── lv_chart（主体）
│   ├── Y 轴刻度 label（手动创建）
│   ├── X 轴标签（手动创建）
│   └── 图例（可选）
└── 点击弹窗查看具体数值
```

### 代码模板
```c
/* 单张 chart */
lv_obj_t * chart = lv_chart_create(parent);
lv_obj_set_size(chart, lv_pct(100), 200);
lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
lv_chart_set_point_count(chart, 20);
lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
lv_chart_set_div_line_count(chart, 4, 3);

lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE),
                                               LV_CHART_AXIS_PRIMARY_Y);
/* 手动创建 Y 轴刻度 label（lv_chart_set_axis_tick 不存在） */
for (int i = 0; i <= 4; i++) {
    lv_obj_t * tick = lv_label_create(chart_container);
    lv_label_set_text_fmt(tick, "%d", 100 - i * 25);
    lv_obj_set_style_text_color(tick, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(tick, &lv_font_montserrat_12, 0);
    lv_obj_align(tick, LV_ALIGN_LEFT_MID, 4, (i * 50) - 100);
}
```

### 组件模式
- `lv_chart_create()` + `lv_chart_set_type(LV_CHART_TYPE_LINE)`
- Y/X 轴标签手动创建（`lv_chart_set_axis_tick` 在当前 LVGL 版本不可用）
- 每张图独立 `lv_chart_series_t *`

### 视觉锚点
- 每条折线独立配色
- 网格线低 opa 辅助读数
- 点击高亮圆点 + 弹出数值

### 常见陷阱
- ❌ `LV_EVENT_CLICKED` 时 `lv_chart_get_pressed_point()` 已被清空 → 改用 `LV_EVENT_PRESSED`
- ❌ 使用 `lv_chart_set_axis_tick` → 不存在此 API，手动创建刻度 label
- ❌ Y 轴标签位置不准 → 用线性插值计算
- ❌ 所有图同一颜色 → 分不清数据线

### 参考
`references/chart-guide.md` — 完整数据图表模板


## §7 对话页 (Chat/AI)

### 识别
用户说"AI 对话"、"聊天界面"、"智能助手"

### 布局策略
```
Flex COLUMN（全屏）
├── 标题栏（固定顶部，不滚，flex_grow=0）
├── 消息列表（flex_grow=1，可滚）
│   ├── AI 消息：左对齐，暗底
│   └── 用户消息：右对齐，accent 底
└── 输入栏（固定底部，flex_grow=0）
    ├── 快捷提问芯片（可选）
    └── 输入框 + 发送按钮
```

### 代码模板
```c
/* 全屏 Flex COLUMN */
lv_obj_t * page = lv_obj_create(parent);
lv_obj_set_size(page, lv_pct(100), lv_pct(100));
lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
NO_SCROLL(page);

/* 标题栏 — 固定高度，不 grow */
lv_obj_t * title_bar = lv_obj_create(page);
lv_obj_set_size(title_bar, lv_pct(100), 56);
lv_obj_set_flex_flow(title_bar, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(title_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_style_pad_hor(title_bar, 16, 0);
NO_SCROLL(title_bar);

/* 消息列表 — flex_grow=1 撑满剩余空间 */
lv_obj_t * msg_list = lv_obj_create(page);
lv_obj_set_size(msg_list, lv_pct(100), LV_SIZE_CONTENT);
lv_obj_set_flex_grow(msg_list, 1);
lv_obj_set_flex_flow(msg_list, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(msg_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_scroll_dir(msg_list, LV_DIR_VER);
lv_obj_set_style_pad_all(msg_list, 16, 0);
lv_obj_set_style_pad_row(msg_list, 12, 0);

/* 单条消息气泡 */
lv_obj_t * bubble = lv_obj_create(msg_list);
lv_obj_set_size(bubble, lv_pct(75), LV_SIZE_CONTENT);
lv_obj_set_style_radius(bubble, 16, 0);
lv_obj_set_style_pad_all(bubble, 12, 0);
NO_SCROLL(bubble);

if (is_user) {
    lv_obj_set_style_bg_color(bubble, lv_color_hex(0x2196F3), 0);  // accent
    lv_obj_set_style_text_color(bubble_label, lv_color_white(), 0);
} else {
    lv_obj_set_style_bg_color(bubble, lv_color_hex(0x1A1D23), 0);  // 暗底
    lv_obj_set_style_text_color(bubble_label, lv_color_hex(0xE0E0E0), 0);
}

lv_obj_t * text = lv_label_create(bubble);
lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);  // 对话页允许 WRAP！
lv_label_set_text(text, message);

/* 输入栏 — 固定底部，不 grow */
lv_obj_t * input_bar = lv_obj_create(page);
lv_obj_set_size(input_bar, lv_pct(100), LV_SIZE_CONTENT);
lv_obj_set_flex_flow(input_bar, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(input_bar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_style_pad_all(input_bar, 12, 0);
lv_obj_set_style_pad_row(input_bar, 8, 0);
lv_obj_set_style_border_width(input_bar, 1, 0);
lv_obj_set_style_border_color(input_bar, lv_color_hex(0x2A2D34), 0);
lv_obj_set_style_border_side(input_bar, LV_BORDER_SIDE_TOP, 0);
NO_SCROLL(input_bar);
```

### 组件模式
- 消息气泡：L1 工厂函数 `create_bubble(parent, text, is_user)`
- 快捷提问：芯片组（同控制面板的模式芯片）

### 视觉锚点
- 用户消息 accent 色 + 右对齐（最突出）
- AI 消息暗底 + 左对齐（低调）
- 输入栏底部固定，有上边框分隔

### 常见陷阱
- ❌ 消息列表没设 flex_grow → 标题栏和输入栏挤在一起
- ❌ 消息列表没设 scroll_dir → 消息多时溢出
- ❌ 气泡 label 用 CLIP → 对话页必须 WRAP
- ❌ **在 flex column 中用 lv_obj_align 定位气泡 → 应用 flex 对齐规则代替**
- ❌ 新消息没自动滚到底 → `lv_obj_scroll_to_view_recursive(new_bubble)`


---

## 场景优先决策树

```
识别用户意图
├── "仪表盘/总览/传感器" → §1 Grid 2×N + sensor_card 工厂函数 (L2)
├── "设置/配置"           → §2 Flex COLUMN 外层可滚 + toggle_row (L3)
├── "列表/日志"           → §3 Flex COLUMN 外层可滚 + list_item (L2)
├── "手表/表盘/圆屏"      → §4 绝对定位 + 指针动画 + 星点 timer
├── "控制/操控/旋钮"      → §5 Grid 不等宽 + Arc + 芯片 + 圆按钮
├── "图表/曲线/趋势"      → §6 lv_chart + 手动刻度 label
└── "对话/聊天/AI"        → §7 Flex COLUMN + flex_grow 消息列表 + 固定输入栏
```

## ⚠️ 代码模板使用须知

1. **这些模板展示的是布局骨架和关键 API 调用顺序**，不是可直接编译的完整代码
2. **所有颜色请用 `lv_color_hex(0xRRGGBB)` 或 `lv_palette_main(LV_PALETTE_XXX)`**
3. **样式初始化必须先 `lv_style_init(&style)` 再 `lv_style_set_*`**
4. **字体需要提前生成**（见 `references/font-pipeline.md`）
5. **组件工厂函数需要自己实现**（见 `references/component-reuse.md`）
6. **真机验证示例见 `references/examples/`** — 这些是完整可编译的代码
