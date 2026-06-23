# Design Token 系统

> **每次生成 UI 时，从此文件选取 token 值，不要临时拍脑袋定数值。**
> 约束选择范围 → 一致性自然发生。

---

## 一、间距 Token

```c
#define SPACE_XS   4    // 图标与文字间距、tight padding
#define SPACE_SM   8    // 卡片内边距（小）、row gap
#define SPACE_MD   12   // 卡片内边距（标准）
#define SPACE_LG   16   // 页面 padding、卡片间距
#define SPACE_XL   24   // 大区域间距
#define SPACE_2XL  32   // 页级间距
```

| Token | px | 适用场景 |
|-------|----|---------|
| `SPACE_XS` | 4 | 图标↔文字、tight 容器内边距 |
| `SPACE_SM` | 8 | 卡片内小控件间距、row gap（紧凑） |
| `SPACE_MD` | 12 | 卡片标准内边距、列 gap |
| `SPACE_LG` | 16 | 页面四周边距、卡片间间距 |
| `SPACE_XL` | 24 | 大区块间距、标题与内容间距 |
| `SPACE_2XL` | 32 | 页面顶部标题区高度、底部安全区 |

**代码中**（token 名仅供选值参考，实际写数字）：
```c
lv_obj_set_style_pad_all(card, 12, 0);       // SPACE_MD
lv_obj_set_style_pad_row(container, 8, 0);   // SPACE_SM
lv_obj_set_style_pad_column(container, 12, 0); // SPACE_MD
```

---

## 二、圆角 Token

| Token | px | 适用场景 |
|-------|----|---------|
| `RADIUS_NONE` | 0 | 工业控制台（硬朗直角风格） |
| `RADIUS_SM` | 4 | 表格单元格、标签芯片 |
| `RADIUS_MD` | 8 | 标准卡片（最常用） |
| `RADIUS_LG` | 12 | 圆润卡片、弹窗、按钮 |
| `RADIUS_XL` | 16 | 家居风格卡片、大面板 |
| `RADIUS_PILL` | 20 | 毛玻璃卡片、pill 按钮 |
| `RADIUS_CIRCLE` | `LV_RADIUS_CIRCLE` | 圆形按钮、icon 容器、头像 |

**代码中**（写具体数值，不写 token 名）：
```c
lv_obj_set_style_radius(card, 8, 0);   // RADIUS_MD — 标准卡片
lv_obj_set_style_radius(btn, 20, 0);   // RADIUS_PILL — pill 按钮
lv_obj_set_style_radius(icon, LV_RADIUS_CIRCLE, 0);  // 圆形
```

---

## 三、阴影预设（Token → LVGL API）

每个预设是完整的 LVGL 参数组合，直接套用。

### 3.1 浮起阴影

| Token | width | offset_y | opa | 视觉效果 |
|-------|-------|----------|-----|---------|
| `SHADOW_NONE` | 0 | 0 | 0 | 无阴影（医疗/扁平风格） |
| `SHADOW_SUBTLE` | 4 | 2 | 26 (10%) | 轻微浮起 |
| `SHADOW_FLOAT` | 12 | 6 | 51 (20%) | 卡片浮起（最常用） |
| `SHADOW_HEAVY` | 20 | 10 | 77 (30%) | 明显浮起（弹窗/模态） |

```c
/* SHADOW_FLOAT — 最常用的卡片浮起 */
lv_obj_set_style_shadow_color(obj, lv_color_hex(0x000000), 0);
lv_obj_set_style_shadow_width(obj, 12, 0);
lv_obj_set_style_shadow_offset_y(obj, 6, 0);
lv_obj_set_style_shadow_offset_x(obj, 0, 0);
lv_obj_set_style_shadow_opa(obj, 51, 0);    // 20%
```

### 3.2 发光 Glow（阴影颜色 = accent）

| Token | width | opa | 视觉效果 |
|-------|-------|-----|---------|
| `GLOW_SOFT` | 15 | 30 | 柔和呼吸光 |
| `GLOW_MODERATE` | 20 | 40 | 明显发光 |
| `GLOW_STRONG` | 30 | 60 | 霓虹灯效果 |
| `GLOW_PULSE` | 20 | 10↔60 | 脉冲呼吸（需 lv_anim 驱动） |

```c
/* GLOW_MODERATE — 强调色发光 */
lv_obj_set_style_shadow_color(obj, accent_color, 0);
lv_obj_set_style_shadow_width(obj, 20, 0);
lv_obj_set_style_shadow_offset_x(obj, 0, 0);
lv_obj_set_style_shadow_offset_y(obj, 0, 0);
lv_obj_set_style_shadow_opa(obj, 102, 0);    // 40%
```

### 3.3 毛玻璃

```c
/* GLASS_STANDARD — iOS 风格毛玻璃卡片 */
lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
lv_obj_set_style_bg_opa(obj, 102, 0);              // 40%
lv_obj_set_style_blur_backdrop(obj, true, 0);
lv_obj_set_style_blur_radius(obj, 18, 0);
lv_obj_set_style_border_width(obj, 1, 0);
lv_obj_set_style_border_color(obj, lv_color_hex(0xFFFFFF), 0);
lv_obj_set_style_border_opa(obj, 77, 0);           // 30%
lv_obj_set_style_radius(obj, 16, 0);   // RADIUS_XL — 大圆角
```

---

## 四、颜色 Token（按 8 种配色方案）

每种风格定义一套完整的颜色角色。**选好配色方案后，严格使用该方案的颜色 token，不要混搭。**

### 风格 1：深海指挥舱 (Deep Sea)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#060E14` | 页面底色 |
| 卡片 | `bg_card` | `#0A1620` | 卡片/面板背景 |
| 卡片悬浮 | `bg_card_hover` | `#0E1C28` | 卡片 pressed/hover |
| 主强调 | `accent` | `#00D4AA` | 按钮/选中态/状态灯 |
| 强调变体 | `accent_dim` | `#008866` | 次要强调 |
| 主文字 | `text_primary` | `#EEEEEE` | 标题/数值 |
| 次文字 | `text_secondary` | `#8899AA` | 辅助信息/单位 |
| 边框 | `border` | `#1A2A38` | 分隔线/卡片边框 |
| 告警 | `danger` | `#FF6B6B` | 错误/危险 |
| 注意 | `warning` | `#FFB347` | 警告/注意 |
| 正常 | `success` | `#00D4AA` | 正常状态（同accent） |

### 风格 2：工业控制台 (Industrial)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#111318` | 页面底色 |
| 卡片 | `bg_card` | `#1A1D23` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#22262E` | 卡片 pressed |
| 主强调 | `accent` | `#FF9800` | 按钮/状态灯 |
| 强调变体 | `accent_dim` | `#CC7A00` | 次要强调 |
| 主文字 | `text_primary` | `#E0E0E0` | 标题/数值 |
| 次文字 | `text_secondary` | `#888888` | 辅助信息 |
| 边框 | `border` | `#2A2D34` | 分隔线 |
| 告警 | `danger` | `#F44336` | 严重告警 |
| 注意 | `warning` | `#FF9800` | 注意（同accent） |
| 正常 | `success` | `#4CAF50` | 正常运行 |

### 风格 3：智能家居 (Smart Home)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#F5F0EB` | 页面底色 |
| 卡片 | `bg_card` | `#FFFFFF` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#F0EBE5` | 卡片 pressed |
| 主强调 | `accent` | `#6B8F71` | 按钮/选中态 |
| 强调变体 | `accent_dim` | `#8FAD94` | 次要强调 |
| 主文字 | `text_primary` | `#2C2C2C` | 标题/数值 |
| 次文字 | `text_secondary` | `#999999` | 辅助信息 |
| 边框 | `border` | `#E0DBD5` | 分隔线 |
| 告警 | `danger` | `#E85D04` | 告警/高温 |
| 注意 | `warning` | `#F4A261` | 注意 |
| 正常 | `success` | `#6B8F71` | 正常（同accent） |

### 风格 4：医疗监护 (Medical)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#F8FAFC` | 页面底色 |
| 卡片 | `bg_card` | `#FFFFFF` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#F1F5F9` | 卡片 pressed |
| 主强调 | `accent` | `#2196F3` | 按钮/选中态 |
| 强调变体 | `accent_dim` | `#64B5F6` | 次要强调 |
| 主文字 | `text_primary` | `#1E293B` | 标题/数值 |
| 次文字 | `text_secondary` | `#64748B` | 辅助信息 |
| 边框 | `border` | `#E2E8F0` | 分隔线 |
| 告警 | `danger` | `#E53935` | 危急值 |
| 注意 | `warning` | `#FFC107` | 临界值 |
| 正常 | `success` | `#4CAF50` | 正常范围 |

### 风格 5：暗色通用 (Dark Universal)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#0D1117` | 页面底色 |
| 卡片 | `bg_card` | `#161B22` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#1C2333` | 卡片 pressed |
| 主强调 | `accent` | `#58A6FF` | 按钮/链接 |
| 强调变体 | `accent_dim` | `#1F6FEB` | 次要强调 |
| 主文字 | `text_primary` | `#E6EDF3` | 标题/数值 |
| 次文字 | `text_secondary` | `#8B949E` | 辅助信息 |
| 边框 | `border` | `#30363D` | 分隔线 |
| 告警 | `danger` | `#F85149` | 错误 |
| 注意 | `warning` | `#D29922` | 注意 |
| 正常 | `success` | `#3FB950` | 正常 |

### 风格 6：奢华高端 (Luxury)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#0A0A0F` | 页面底色 |
| 卡片 | `bg_card` | `#1A1A24` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#24243A` | 卡片 pressed |
| 主强调 | `accent` | `#C9A96E` | 金色强调 |
| 强调变体 | `accent_dim` | `#8B734B` | 暗金 |
| 主文字 | `text_primary` | `#E8E0D0` | 标题/数值 |
| 次文字 | `text_secondary` | `#8A8070` | 辅助信息 |
| 边框 | `border` | `#2A2A38` | 分隔线 |
| 告警 | `danger` | `#8B0000` | 暗红告警 |
| 注意 | `warning` | `#C9A96E` | 注意（同accent） |
| 正常 | `success` | `#7B9B6A` | 暗绿正常 |

### 风格 7：赛博朋克 (Cyberpunk)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#0D0221` | 页面底色 |
| 卡片 | `bg_card` | `#1A0A2E` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#250F40` | 卡片 pressed |
| 主强调 | `accent` | `#FF00FF` | 品红强调 |
| 强调变体 | `accent_dim` | `#CC00CC` | 暗品红 |
| 辅强调 | `accent_alt` | `#00FFFF` | 青色（第二强调色） |
| 主文字 | `text_primary` | `#F0E0FF` | 标题/数值 |
| 次文字 | `text_secondary` | `#8A70B0` | 辅助信息 |
| 边框 | `border` | `#3A2060` | 分隔线 |
| 告警 | `danger` | `#FF00FF` | 告警（同accent） |
| 注意 | `warning` | `#FFD700` | 金色注意 |
| 正常 | `success` | `#00FF88` | 霓虹绿正常 |

### 风格 8：自然户外 (Nature)

| 角色 | Token 名 | 色值 | 用途 |
|------|---------|------|------|
| 背景底 | `bg_base` | `#1B2E1E` | 页面底色 |
| 卡片 | `bg_card` | `#243828` | 卡片背景 |
| 卡片悬浮 | `bg_card_hover` | `#2D4432` | 卡片 pressed |
| 主强调 | `accent` | `#4CAF50` | 绿色强调 |
| 强调变体 | `accent_dim` | `#388E3C` | 暗绿 |
| 主文字 | `text_primary` | `#E8F0E8` | 标题/数值 |
| 次文字 | `text_secondary` | `#8AA88A` | 辅助信息 |
| 边框 | `border` | `#2A3E2A` | 分隔线 |
| 告警 | `danger` | `#FF7043` | 橙红告警 |
| 注意 | `warning` | `#FFB74D` | 琥珀注意 |
| 正常 | `success` | `#81C784` | 浅绿正常 |

---

## 五、字体层级 Token

| Token | Montserrat | 中文/图标字体 | 适用场景 |
|-------|-----------|-------------|---------|
| `FONT_XS` | 14px | 14px | 辅助信息、单位、时间戳 |
| `FONT_SM` | 16px | 16px | 正文、标签、按钮文字 |
| `FONT_MD` | 20px | 20px | 标题、设备名 |
| `FONT_LG` | 24px | 24px | 页面大标题、核心数值 |
| `FONT_XL` | 32px | 32px | 超大数值（仪表盘核心数据） |
| `FONT_XXL` | 48px | - | 时间显示（手表） |

---

## 六、边框预设

| Token | width | opa | 适用场景 |
|-------|-------|-----|---------|
| `BORDER_NONE` | 0 | 0 | 无边框（大多数情况） |
| `BORDER_SUBTLE` | 1 | 20% | 轻分隔线 |
| `BORDER_CARD` | 1 | 30% | 卡片边框（暗色主题用） |
| `BORDER_ACCENT` | 2 | 40% | accent 色边框（强调态） |
| `BORDER_GLOW` | 2 | 60% | 发光边框（霓虹/赛博） |

---

## 七、使用原则

1. **选一个配色方案就只用一个** — 不要混搭两个方案的 token
2. **间距从 token 表中选** — 不要写 `lv_dpx(7)` 或 `lv_dpx(13)` 这种不在表中的值
3. **阴影从预设中选** — 不要自己临时拼 shadow 参数
4. **圆角统一** — 同一个页面内所有卡片用同一个 `RADIUS_*` token，除非有明确的视觉理由
5. **暗色主题默认带 `BORDER_CARD`** — 暗底上卡片没有边框会融进背景
6. **亮色主题默认 `SHADOW_SUBTLE` 或 `SHADOW_FLOAT`** — 亮底需要阴影区分层次
