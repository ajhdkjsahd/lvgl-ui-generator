# LVGL v9 视觉特效完整目录

> 30+ 种视觉特效的完整代码配方。每个配方可直接复制使用。
> 需要启用 `LV_USE_DRAW_SW_COMPLEX_GRADIENTS` 以使用复杂渐变。

---

## 一、光影特效

### 1.1 Box Shadow — 卡片浮起

```c
lv_style_set_bg_color(&style, lv_color_hex(0xFFFFFF));
lv_style_set_radius(&style, 12);
lv_style_set_shadow_color(&style, lv_color_hex(0x000000));
lv_style_set_shadow_width(&style, 20);
lv_style_set_shadow_offset_y(&style, 8);
lv_style_set_shadow_opa(&style, LV_OPA_20);
```

### 1.2 Colored Glow — 霓虹发光

```c
lv_style_set_shadow_color(&style, accent_color);   // 用强调色做阴影
lv_style_set_shadow_width(&style, 30);              // 大扩散
lv_style_set_shadow_offset_x(&style, 0);
lv_style_set_shadow_offset_y(&style, 0);
lv_style_set_shadow_opa(&style, LV_OPA_40);         // 40% 不透明度
```

### 1.3 Drop Shadow — 形状感知投影（弧形/不规则形状）

```c
lv_style_set_drop_shadow_color(&style, lv_color_hex(0x000000));
lv_style_set_drop_shadow_radius(&style, 16);         // 模糊半径
lv_style_set_drop_shadow_opa(&style, LV_OPA_30);
lv_style_set_drop_shadow_offset_x(&style, 4);
lv_style_set_drop_shadow_offset_y(&style, 8);
```

### 1.4 Frosted Glass — 毛玻璃/磨砂（iOS 风格）

```c
lv_style_set_bg_color(&style, lv_color_hex(0xFFFFFF));
lv_style_set_bg_opa(&style, (255 * 40 / 100));       // 40% 背景透明
lv_style_set_border_width(&style, 1);
lv_style_set_border_color(&style, lv_color_hex(0xFFFFFF));
lv_style_set_border_opa(&style, (255 * 60 / 100));   // 60% 边框
lv_style_set_radius(&style, 20);
lv_style_set_blur_backdrop(&style, true);             // 背景模糊
lv_style_set_blur_radius(&style, 18);                 // 模糊强度
lv_style_set_text_color(&style, lv_color_hex(0xFFFFFF));
```

### 1.5 3D Button — 按下时下沉

```c
// 默认态：浮起
lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));
lv_style_set_bg_grad_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 2));
lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);     // 上浅下深 = 凸起
lv_style_set_shadow_width(&style, 8);
lv_style_set_shadow_offset_y(&style, 4);
lv_style_set_translate_y(&style, 0);

// 按下态：下沉
lv_style_set_bg_grad_dir(&style_pr, LV_GRAD_DIR_VER);  // 反转渐变 = 凹下
lv_style_set_shadow_width(&style_pr, 2);
lv_style_set_shadow_offset_y(&style_pr, 1);
lv_style_set_translate_y(&style_pr, 3);                 // 整体下移
lv_obj_add_style(btn, &style, 0);
lv_obj_add_style(btn, &style_pr, LV_STATE_PRESSED);
```

---

## 二、渐变特效

### 2.1 简单水平渐变按钮

```c
lv_style_set_bg_color(&style, lv_color_hex(0x6366F1));
lv_style_set_bg_grad_color(&style, lv_color_hex(0x8B5CF6));
lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_HOR);
lv_style_set_radius(&style, 10);
```

### 2.2 Vignette 暗角 — 整个屏幕径向渐变

```c
static const lv_color_t grad_colors[2] = {
    LV_COLOR_MAKE(0x0B, 0x0B, 0x2B),   // 边缘深色
    LV_COLOR_MAKE(0x1A, 0x1A, 0x3E),   // 中心稍亮
};
static lv_grad_dsc_t grad;
lv_grad_init_stops(&grad, grad_colors, NULL, NULL, 2);
lv_grad_radial_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER,
                    LV_GRAD_RIGHT, LV_GRAD_BOTTOM, LV_GRAD_EXTEND_PAD);
lv_style_set_bg_grad(&style_bg, &grad);
```

### 2.3 金属拉丝 — 锥形渐变

```c
static const lv_color_t metal_colors[8] = {
    LV_COLOR_MAKE(0xE8, 0xE8, 0xE8), LV_COLOR_MAKE(0xFF, 0xFF, 0xFF),
    LV_COLOR_MAKE(0xFA, 0xFA, 0xFA), LV_COLOR_MAKE(0x79, 0x79, 0x79),
    LV_COLOR_MAKE(0x48, 0x48, 0x48), LV_COLOR_MAKE(0x4B, 0x4B, 0x4B),
    LV_COLOR_MAKE(0x70, 0x70, 0x70), LV_COLOR_MAKE(0xE8, 0xE8, 0xE8),
};
static lv_grad_dsc_t grad;
lv_grad_init_stops(&grad, metal_colors, NULL, NULL, 8);
lv_grad_conical_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER,
                     0, 120, LV_GRAD_EXTEND_REFLECT);
lv_style_set_bg_grad(&style, &grad);
lv_style_set_radius(&style, LV_RADIUS_CIRCLE);
lv_style_set_shadow_width(&style, 30);     // 加深阴影增强立体感
lv_style_set_shadow_color(&style, lv_color_black());
```

### 2.4 多色渐变 — 极光效果

```c
static const lv_color_t aurora_colors[4] = {
    LV_COLOR_MAKE(0x06, 0x0E, 0x14),   // 深海蓝
    LV_COLOR_MAKE(0x00, 0xD4, 0xAA),   // 生物荧光青
    LV_COLOR_MAKE(0x8B, 0x5C, 0xF6),   // 紫
    LV_COLOR_MAKE(0x06, 0x0E, 0x14),   // 回到深海蓝
};
static lv_grad_dsc_t grad;
lv_grad_init_stops(&grad, aurora_colors, NULL, NULL, 4);
lv_grad_linear_init(&grad, lv_pct(0), lv_pct(0),
                    lv_pct(100), lv_pct(100), LV_GRAD_EXTEND_REFLECT);
lv_style_set_bg_grad(&style, &grad);
```

### 2.5 Gradient Stops 精确控制 — 渐变位置

```c
// bg_main_stop: 主色在渐变中的结束位置 (0~255)
// bg_grad_stop: 渐变色在渐变中的开始位置 (0~255)
lv_style_set_bg_color(&style, lv_color_hex(0x6366F1));
lv_style_set_bg_grad_color(&style, lv_color_hex(0xEC4899));
lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);
lv_style_set_bg_main_stop(&style, 80);    // 前 80/255 纯 indigo
lv_style_set_bg_grad_stop(&style, 220);   // 后 35/255 纯 pink，中间渐变
```

---

## 三、质感特效

### 3.1 Blend Mode — 叠加发光

```c
lv_style_set_bg_color(&style, accent);
lv_style_set_bg_opa(&style, LV_OPA_20);
lv_style_set_blend_mode(&style, LV_BLEND_MODE_ADDITIVE);  // 叠加到下层
```

### 3.2 Multiply — 正片叠底暗角

```c
lv_style_set_bg_color(&style, lv_color_black());
lv_style_set_bg_opa(&style, LV_OPA_30);
lv_style_set_blend_mode(&style, LV_BLEND_MODE_MULTIPLY);
```

### 3.3 Image Recoloring — 图标随主题变色

```c
lv_style_set_image_recolor(&style, accent_color);
lv_style_set_image_recolor_opa(&style, LV_OPA_COVER);
// 配合白色图标 → 变成任意颜色
```

### 3.4 Bitmap Mask — 异形裁剪

```c
LV_IMAGE_DECLARE(mask_pattern);  // A8 格式位图
lv_style_set_bitmap_mask_src(&style, &mask_pattern);
// 整个 widget 被 mask 形状裁剪
```

### 3.5 Pulsing Glow — 脉冲呼吸光晕

```c
static void pulse_anim_cb(void * var, int32_t v) {
    lv_obj_set_style_shadow_opa((lv_obj_t*)var, v, 0);
}
void anim_pulse_glow(lv_obj_t * obj, lv_color_t color) {
    lv_obj_set_style_shadow_color(obj, color, 0);
    lv_obj_set_style_shadow_width(obj, 20, 0);
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, pulse_anim_cb);
    lv_anim_set_values(&a, LV_OPA_10, LV_OPA_60);
    lv_anim_set_duration(&a, 1500);
    lv_anim_set_reverse_duration(&a, 1500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}
```

### 3.6 Opacity Layering — 半透明景深

```c
// 背景卡：低透明度
lv_obj_set_style_bg_opa(card_bg, LV_OPA_10, 0);
// 前景卡：高透明度
lv_obj_set_style_bg_opa(card_fg, LV_OPA_80, 0);
// 形成景深层次
```

---

## 四、变换特效

### 4.1 指针旋转（表盘/仪表）

```c
lv_image_set_pivot(needle, pivot_x, pivot_y);   // 旋转中心点
lv_image_set_rotation(needle, angle * 10);       // 角度×10（0.1°单位）
// 动画:
lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_image_set_rotation);
```

### 4.2 Pop-in 弹入动画

```c
lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_scale_x);
lv_anim_set_values(&a, 0, 256);      // 从 0 到 100%
lv_anim_set_path_cb(&a, lv_anim_path_overshoot);  // 过冲回弹
```

### 4.3 圆形排列（translate_radial）

```c
// 子元素围绕父中心排列
lv_obj_set_style_translate_radial(child, radius, 0);
lv_obj_set_style_transform_pivot_x(child, lv_pct(50), 0);
lv_obj_set_style_transform_rotation(child, angle_per_item * i * 10, 0);
```

---

## 五、自定义绘制

### 5.1 Draw Task — 自定义形状叠加

```c
static void draw_event_cb(lv_event_t * e) {
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);
    if(base_dsc->part != LV_PART_MAIN) return;

    // 画一个圆
    lv_draw_rect_dsc_t circle_dsc;
    lv_draw_rect_dsc_init(&circle_dsc);
    circle_dsc.bg_color = lv_color_hex(0xFF0000);
    circle_dsc.radius = LV_RADIUS_CIRCLE;

    lv_area_t a;
    a.x1 = 0; a.y1 = 0; a.x2 = 40; a.y2 = 40;
    lv_area_t obj_coords;
    lv_obj_get_coords(obj, &obj_coords);
    lv_area_align(&obj_coords, &a, LV_ALIGN_CENTER, 0, 0);
    lv_draw_rect(base_dsc->layer, &circle_dsc, &a);
}

lv_obj_add_event_cb(obj, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
lv_obj_add_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
```

### 5.2 Draw Main — 自定义背景绘制

```c
static void main_draw_cb(lv_event_t * e) {
    lv_layer_t * layer = lv_event_get_layer(e);
    // 画三角形
    lv_draw_triangle_dsc_t tri_dsc;
    lv_draw_triangle_dsc_init(&tri_dsc);
    tri_dsc.p[0].x = 100; tri_dsc.p[0].y = 200;
    tri_dsc.p[1].x = 150; tri_dsc.p[1].y = 150;
    tri_dsc.p[2].x = 200; tri_dsc.p[2].y = 200;
    tri_dsc.color = lv_color_hex(0x00D4AA);
    lv_draw_triangle(layer, &tri_dsc);
}
lv_obj_add_event_cb(obj, main_draw_cb, LV_EVENT_DRAW_MAIN_BEGIN, NULL);
```

---

## 六、动画组合模式

### 6.1 Staggered 逐项入场

```c
for(int i = 0; i < count; i++) {
    lv_obj_set_style_opa(items[i], LV_OPA_TRANSP, 0);
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, items[i]);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a, 300);
    lv_anim_set_delay(&a, i * 80);  // 每项延迟 80ms
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}
```

### 6.2 Counter 数值滚动

```c
// 参考 references/animation-guide.md 完整实现
// 核心：lv_anim 驱动进度 0→1000，映射到实际数值范围
```

### 6.3 Timeline 时间线编排

```c
lv_anim_timeline_t * at = lv_anim_timeline_create();
lv_anim_timeline_add(at, 0, &anim1);      // 0ms 开始
lv_anim_timeline_add(at, 200, &anim2);    // 200ms 开始
lv_anim_timeline_add(at, 400, &anim3);    // 400ms 开始
lv_anim_timeline_start(at);
```

---

## 七、组合设计配方

### 7.1 "深空" — 表盘/仪表背景
```
径向渐变(暗蓝→黑) + 星点自定义绘制 + 锥形渐变金属环
```

### 7.2 "霓虹都市" — 赛博朋克面板
```
暗紫底色 + ADDITIVE 叠加发光边框 + glow 阴影数字 + 品红强调色
```

### 7.3 "自然大地" — 户外运动面板
```
深绿底色 + 径向渐变模拟阳光 + 大圆角卡片 + 柔和阴影
```

### 7.4 "至臻奢华" — 高端商务面板
```
深黑底色 + 金色锥形渐变装饰 + 细线分隔 + 低透明度阴影层次
```

### 7.5 "极简白" — 医疗/实验面板
```
冷白底色 + 浅灰 border 分隔 + 蓝色 accent + 无阴影（扁平）
```

### 7.6 "海洋深渊" — 水产/海洋面板
```
深渊黑底 + 生物荧光青 glow + 珊瑚红告警 + 深海蓝渐变卡片
```

---

## 八、快速参数速查

| 效果 | 关键参数 | 典型值 |
|------|---------|--------|
| 轻微浮起 | shadow_width + offset_y | 10 + 4 |
| 明显浮起 | shadow_width + offset_y | 20 + 8 |
| 霓虹发光 | shadow_width + opa | 30 + 40 |
| 毛玻璃 | blur_radius + bg_opa | 18 + 40% |
| 3D 凸起 | bg_grad_dir=VER + shadow | 上浅下深 |
| 3D 凹下 | bg_grad_dir=VER(反) - translate_y | 上深下浅 + 下移 |
| 金属光泽 | conical + REFLECT + 6-8 stops | 多灰度交替 |
| 深空暗角 | radial + 边缘深色 + 中心亮 | LV_GRAD_EXTEND_PAD |
| 光束 | linear + 半透明彩色 + REFLECT | 角度倾斜 |
