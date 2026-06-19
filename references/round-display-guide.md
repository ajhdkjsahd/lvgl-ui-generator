# 圆屏手表 UI 设计专项指南

> 覆盖圆屏坐标系、表盘层级、指针动画、环形布局等所有圆屏手表设计核心技术。

---

## 一、圆屏坐标系统

### 1.1 基本概念

| 概念 | LVGL API | 说明 |
|------|---------|------|
| 圆心 | `lv_obj_center(obj)` 或 `LV_ALIGN_CENTER` | 通常对齐到容器中心 |
| 半径 | `lv_obj_set_size(obj, r*2, r*2)` | 控件尺寸 = 直径 |
| 角度 | `transform_rotation` (0.1°单位) | 360° = 3600 |
| 锚点 | `transform_pivot_x/y` | 旋转/缩放中心 |
| 极轴偏移 | `translate_radial` | 沿半径方向移动 |
| 圆形裁剪 | `radius = LV_RADIUS_CIRCLE` | `lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0)` |

### 1.2 圆形裁剪

```c
// 让任意容器变成圆形视口
lv_obj_set_style_radius(circle_container, LV_RADIUS_CIRCLE, 0);
lv_obj_set_style_clip_corner(circle_container, true, 0);  // 裁剪子元素
```

### 1.3 极坐标定位

```c
// 围绕圆心排列子元素
lv_obj_set_style_translate_radial(child, radius, 0);        // 离圆心距离
lv_obj_set_style_transform_rotation(child, angle * 10, 0);  // 角度
lv_obj_set_style_transform_pivot_x(child, lv_pct(50), 0);   // 自转中心
lv_obj_set_style_transform_pivot_y(child, lv_pct(50), 0);
```

---

## 二、表盘层级结构（从底层到顶层）

```
Layer 0: 背景层（深空渐变 / 图片 / 纯色）
Layer 1: 装饰层（星点动画 / 极光渐变 / 纹理）
Layer 2: 刻度环（lv_scale + ROUND_OUTER/INNER + Sections）
Layer 3: 信息层（日期 / 步数 / 天气 / 电池）
Layer 4: 指针层（时针 / 分针 / 秒针 lv_image + rotation）
Layer 5: 中心轴盖（小圆 + conical gradient + 阴影）
Layer 6: 浮层（通知弹窗 / 菜单）
```

### 2.1 表盘层级代码模板

```c
lv_obj_t * create_watch_face(lv_obj_t * parent) {
    // 创建圆形表盘容器
    lv_obj_t * face = lv_obj_create(parent);
    lv_obj_set_size(face, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(face, true, 0);
    lv_obj_set_style_border_width(face, 0, 0);
    lv_obj_set_style_pad_all(face, 0, 0);
    lv_obj_center(face);

    // Layer 0: 背景
    create_watch_bg(face);

    // Layer 2: 刻度环
    create_watch_scale(face);

    // Layer 3: 信息
    create_watch_info(face);

    // Layer 4: 指针
    create_watch_hands(face);

    // Layer 5: 中心盖
    create_center_cap(face);

    return face;
}
```

---

## 三、表盘背景

### 3.1 深空径向渐变背景

```c
static void create_watch_bg(lv_obj_t * parent) {
    lv_obj_t * bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(bg, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    lv_obj_set_style_pad_all(bg, 0, 0);
    lv_obj_center(bg);

    static const lv_color_t space_colors[2] = {
        LV_COLOR_MAKE(0x0B, 0x12, 0x2E),   // 边缘深蓝
        LV_COLOR_MAKE(0x1A, 0x2A, 0x4A),   // 中心稍亮
    };
    static lv_grad_dsc_t grad;
    lv_grad_init_stops(&grad, space_colors, NULL, NULL, 2);
    lv_grad_radial_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER,
                        LV_GRAD_RIGHT, LV_GRAD_BOTTOM, LV_GRAD_EXTEND_PAD);
    lv_obj_set_style_bg_grad(bg, &grad, 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
}
```

### 3.2 星点闪烁装饰

```c
typedef struct {
    lv_point_t pos;
    uint8_t brightness;
    int8_t dir;
    lv_obj_t * dot;
} star_t;

static star_t stars[80];

static void create_stars(lv_obj_t * parent) {
    int32_t r = lv_obj_get_width(parent) / 2 - 10;
    for(int i = 0; i < 80; i++) {
        float angle = (float)(lv_rand(0, 3600)) / 10.0f;
        float dist = lv_rand(r * 30 / 100, r);
        stars[i].pos.x = (int32_t)(cos(angle * 3.14159f / 180) * dist);
        stars[i].pos.y = (int32_t)(sin(angle * 3.14159f / 180) * dist);
        stars[i].brightness = lv_rand(20, 200);
        stars[i].dir = (lv_rand(0, 1) ? 1 : -1);

        stars[i].dot = lv_obj_create(parent);
        lv_obj_set_size(stars[i].dot, 2, 2);
        lv_obj_set_style_radius(stars[i].dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(stars[i].dot, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(stars[i].dot, stars[i].brightness, 0);
        lv_obj_set_style_border_width(stars[i].dot, 0, 0);
        lv_obj_set_pos(stars[i].dot, r + stars[i].pos.x, r + stars[i].pos.y);
    }
}

// 用 lv_timer 周期性更新星点亮度和位置（模拟闪烁）
static void star_timer_cb(lv_timer_t * t) {
    for(int i = 0; i < 80; i++) {
        stars[i].brightness += stars[i].dir * lv_rand(2, 10);
        if(stars[i].brightness > 200) stars[i].dir = -1;
        if(stars[i].brightness < 10) stars[i].dir = 1;
        lv_obj_set_style_bg_opa(stars[i].dot, stars[i].brightness, 0);
    }
}
```

---

## 四、刻度环（lv_scale）

### 4.1 圆形刻度环模板

```c
static lv_obj_t * create_watch_scale(lv_obj_t * parent) {
    lv_obj_t * scale = lv_scale_create(parent);
    lv_obj_set_size(scale, lv_pct(100), lv_pct(100));
    lv_obj_center(scale);

    // 模式：圆环外侧
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_scale_set_range(scale, 0, 60);         // 60 分钟
    lv_scale_set_total_tick_count(scale, 61);  // 60+1 个 tick
    lv_scale_set_major_tick_every(scale, 5);   // 每 5 分钟粗刻度
    lv_scale_set_angle_range(scale, 360);      // 完整 360°
    lv_scale_set_rotation(scale, 0);

    // 粗刻度样式（INDICATOR = major ticks）
    lv_obj_set_style_length(scale, 12, LV_PART_INDICATOR);    // 刻度长度
    lv_obj_set_style_line_width(scale, 2, LV_PART_INDICATOR);  // 刻度线宽
    lv_obj_set_style_line_color(scale, lv_color_hex(0xCCDDFF), LV_PART_INDICATOR);

    // 细刻度样式（ITEMS = minor ticks）
    lv_obj_set_style_length(scale, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_width(scale, 1, LV_PART_ITEMS);
    lv_obj_set_style_line_color(scale, lv_color_hex(0x556688), LV_PART_ITEMS);

    // 主弧线样式（MAIN）
    lv_obj_set_style_arc_color(scale, lv_color_hex(0x334466), LV_PART_MAIN);
    lv_obj_set_style_arc_width(scale, 2, LV_PART_MAIN);

    return scale;
}
```

### 4.2 分区刻度（红区/绿区）

```c
// 心率区间：绿区(60-100) + 橙区(100-140) + 红区(140-180)
static void create_hr_scale_sections(lv_obj_t * scale) {
    lv_style_t style_green_main, style_orange_main, style_red_main;
    lv_style_init(&style_green_main);
    lv_style_set_arc_color(&style_green_main, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_arc_width(&style_green_main, 6);
    // ... orange, red styles ...

    lv_scale_section_t * sec;
    sec = lv_scale_add_section(scale);
    lv_scale_set_section_range(scale, sec, 60, 100);
    lv_scale_set_section_style_main(scale, sec, &style_green_main);

    sec = lv_scale_add_section(scale);
    lv_scale_set_section_range(scale, sec, 100, 140);
    lv_scale_set_section_style_main(scale, sec, &style_orange_main);

    sec = lv_scale_add_section(scale);
    lv_scale_set_section_range(scale, sec, 140, 180);
    lv_scale_set_section_style_main(scale, sec, &style_red_main);
}
```

---

## 五、指针系统

### 5.1 指针旋转核心 API

```c
// 旋转中心设为指针根部（图片底部中央）
lv_image_set_pivot(hand, img_width / 2, img_height - 5);
// 旋转角度（0.1°单位，360°=3600）
lv_image_set_rotation(hand, angle * 10);
```

### 5.2 三层指针创建

```c
typedef struct {
    lv_obj_t * hour;
    lv_obj_t * minute;
    lv_obj_t * second;
} watch_hands_t;

static watch_hands_t create_hands(lv_obj_t * parent) {
    watch_hands_t hands;

    // 时针：金色，粗短
    hands.hour = lv_image_create(parent);
    LV_IMAGE_DECLARE(img_hand_hour);
    lv_image_set_src(hands.hour, &img_hand_hour);
    lv_image_set_pivot(hands.hour, 4, 80);    // 根部在图片底
    lv_obj_center(hands.hour);
    // 金色 glow
    lv_obj_set_style_shadow_color(hands.hour, lv_color_hex(0xC9A96E), 0);
    lv_obj_set_style_shadow_width(hands.hour, 12, 0);
    lv_obj_set_style_shadow_opa(hands.hour, LV_OPA_40, 0);

    // 分针：银色，细长
    hands.minute = lv_image_create(parent);
    LV_IMAGE_DECLARE(img_hand_minute);
    lv_image_set_src(hands.minute, &img_hand_minute);
    lv_image_set_pivot(hands.minute, 3, 110);
    lv_obj_center(hands.minute);

    // 秒针：红色，极细
    hands.second = lv_image_create(parent);
    LV_IMAGE_DECLARE(img_hand_second);
    lv_image_set_src(hands.second, &img_hand_second);
    lv_image_set_pivot(hands.second, 2, 120);
    lv_obj_center(hands.second);
    // 红色光晕
    lv_obj_set_style_shadow_color(hands.second, lv_color_hex(0xFF4050), 0);
    lv_obj_set_style_shadow_width(hands.second, 8, 0);
    lv_obj_set_style_shadow_opa(hands.second, LV_OPA_50, 0);

    return hands;
}
```

### 5.3 指针更新（1 秒定时器）

```c
static void update_hands_timer_cb(lv_timer_t * t) {
    watch_hands_t * hands = (watch_hands_t*)t->user_data;

    // 获取当前时间（用系统 RTC 或模拟时间）
    int h = 10, m = 42, s = 35;  // 示例: 10:42:35

    // 计算角度
    int32_t sec_angle = s * 60;           // 0.1°: 60 * 6 = 360°/60秒 → 每步6°
    int32_t min_angle = m * 60 + s;       // 每分钟 6° + 每秒微量
    int32_t hour_angle = (h % 12) * 300 + m * 5;  // 每小时 30° + 每分钟 0.5°

    // 平滑动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    lv_anim_set_var(&a, hands->second);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_image_set_rotation);
    lv_anim_set_values(&a, lv_image_get_rotation(hands->second), sec_angle);
    lv_anim_start(&a);

    // 分针和时针同理...
}
```

### 5.4 不用图片的纯代码指针

```c
// 用 lv_obj + transform 画指针（不需要准备图片资源）
static lv_obj_t * create_code_hand(lv_obj_t * parent, lv_color_t color,
                                    int32_t width, int32_t length) {
    lv_obj_t * hand = lv_obj_create(parent);
    lv_obj_set_size(hand, width, length);
    lv_obj_set_style_radius(hand, width / 2, 0);
    lv_obj_set_style_bg_color(hand, color, 0);
    lv_obj_set_style_border_width(hand, 0, 0);

    // 锚点在底部中央
    lv_obj_set_style_transform_pivot_x(hand, lv_pct(50), 0);
    lv_obj_set_style_transform_pivot_y(hand, lv_pct(100), 0);

    return hand;
}
```

---

## 六、中心轴盖

```c
static lv_obj_t * create_center_cap(lv_obj_t * parent) {
    lv_obj_t * cap = lv_obj_create(parent);
    lv_obj_set_size(cap, 16, 16);
    lv_obj_set_style_radius(cap, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(cap, 0, 0);
    lv_obj_center(cap);

    // 金属锥形渐变
    static const lv_color_t cap_colors[4] = {
        LV_COLOR_MAKE(0xFF, 0xFF, 0xFF),
        LV_COLOR_MAKE(0xC9, 0xA9, 0x6E),
        LV_COLOR_MAKE(0x8B, 0x73, 0x4B),
        LV_COLOR_MAKE(0xC9, 0xA9, 0x6E),
    };
    static lv_grad_dsc_t grad;
    lv_grad_init_stops(&grad, cap_colors, NULL, NULL, 4);
    lv_grad_conical_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER,
                         0, 90, LV_GRAD_EXTEND_REFLECT);
    lv_obj_set_style_bg_grad(cap, &grad, 0);

    // 深度阴影
    lv_obj_set_style_shadow_color(cap, lv_color_black(), 0);
    lv_obj_set_style_shadow_width(cap, 8, 0);
    lv_obj_set_style_shadow_offset_y(cap, 2, 0);
    lv_obj_set_style_shadow_opa(cap, LV_OPA_50, 0);

    return cap;
}
```

---

## 七、表盘类型设计配方

### 7.1 深空极光表盘（奢华科技感）
```
背景: 径向渐变(深蓝→黑) + 60-80颗星点闪烁
表圈: 锥形渐变金属环 + 深阴影
刻度: ROUND_OUTER 细白线 + 5分粗刻度
指针: 金色时针/银色分针/红色秒针 + 各自 glow 光晕
中心盖: 金色锥形渐变
浮层: 毛玻璃(blur_backdrop)日期窗
```

### 7.2 极简数字表盘（现代商务）
```
背景: 纯黑 + 微妙径向渐变
刻度: 无（纯数字）
时间: FreeType 大数码字体(56px) + text_outline_stroke
信息: 小字日期/步数/电量 icon 行
无指针，纯数字
```

### 7.3 运动数据表盘（户外健身）
```
背景: 深绿→黑径向渐变
进度环: 多层 Arc（步数/心率/卡路里）各不同颜色
分区: Scale Sections 红/绿心率区间
大数字: 步数计数 + 距离
指针: 无（数据环为中心）
```

### 7.4 复古机械表盘（经典优雅）
```
背景: 象牙白/米色
表圈: 棕色皮革纹理（图片）
刻度: 罗马数字（自定义 labels）+ ROUND_OUTER
指针: 柳叶形指针（图片）+ 轻微阴影
中心盖: 纹理金属
日期窗: 3点位小方窗
```

---

## 八、手势交互

### 8.1 左右滑切换表盘

```c
static void on_gesture(lv_event_t * e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if(dir == LV_DIR_LEFT) {
        // 切换到上一个表盘
        switch_watchface(current_face - 1);
    } else if(dir == LV_DIR_RIGHT) {
        // 切换到下一个表盘
        switch_watchface(current_face + 1);
    } else if(dir == LV_DIR_BOTTOM) {
        // 上滑展开通知/控制中心
        show_control_panel();
    }
}

lv_obj_add_event_cb(watch_face, on_gesture, LV_EVENT_GESTURE, NULL);
lv_obj_add_flag(watch_face, LV_OBJ_FLAG_CLICKABLE);
```

### 8.2 表盘切换动画

```c
static void switch_watchface(int new_id) {
    lv_obj_t * old = watch_faces[current_id];
    lv_obj_t * new = watch_faces[new_id];

    // 淡出旧表盘
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, old);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    // 淡入新表盘
    lv_obj_set_style_opa(new, LV_OPA_TRANSP, 0);
    lv_anim_set_var(&a, new);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a, 300);
    lv_anim_set_delay(&a, 150);
    lv_anim_start(&a);

    current_id = new_id;
}
```

---

## 九、社区标杆项目（设计灵感来源）

| 项目 | 平台 | 亮点设计 |
|------|------|---------|
| **ZSWatch** (3.3k⭐) | nRF5340+Zephyr | 完整开源手表，多表盘切换架构，BLE通知UI |
| **OV-Watch** (2.3k⭐) | STM32+FreeRTOS | 多风格表盘，传感器数据可视化 |
| **HoloCubic** (1k⭐) | ESP32+Arduino | 240x240极小圆屏高密度信息设计，GIF播放 |
| **X-Knob** (864⭐) | ESP32 | 圆形旋钮UI，环形菜单，MQTT智能家居面板 |
| **WatchX** (905⭐) | STM32F4 | 经典LittlevGL手表，多应用切换 |
| **IceNav-v3** (388⭐) | ESP32-S3 | GPS离线地图导航，LVGL渲染OSM瓦片地图 |
| **X-TRACK** (6.2k⭐) | ESP32 | GPS码表，复杂数据仪表盘，轨迹记录 |
| **project_aura** (667⭐) | ESP32-S3 | 空气质量站，传感器图表，HomeAssistant集成 |

**设计启示**：
- 真实的表盘产品都支持**多表盘切换**（左右滑手势），不是单一表盘
- 圆形屏幕的信息布局要在中心区域留白给指针，边缘环形区域放信息
- 极小屏幕(≤240px)需要精简到 2-3 个信息维度，字体层级不超过 2 级
- 通知系统是手表核心交互，需要 Toast + 通知列表 + 振动反馈

---

## 十、性能注意事项（嵌入式手表）

| 操作 | MCU 建议 |
|------|---------|
| 表盘背景（静态） | 只渲染一次，不每帧刷新 |
| 指针旋转 | 每秒最多 1 次（秒针），小时级微量更新 |
| 星点闪烁 | 用 lv_timer 200ms 周期，每次只更新亮度，不重绘 |
| 渐变背景 | 静态不使用 animation；如需动画用 `transform_rotation` 旋转整个渐变层 |
| 同时动画数 | ≤ 3 个（秒针平滑 + 1 个信息更新 + 1 个呼吸效果） |
| 字体 | 用预生成位图字体（lv_font_conv），不用 Tiny TTF 运行时解析 |
| 图片 | 用 RGB565 格式 C 数组，不用文件系统加载 |
| 表盘切换 | Fade 动画控制在 200-300ms |
