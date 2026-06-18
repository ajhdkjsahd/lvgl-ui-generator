# 动画系统实用指南

LVGL v9 `lv_anim_t` 提供属性动画——对任意数值在指定时间内平滑过渡。API 详见 `references/lvgl-v9-api-cheatsheet.md` §6。

---

## 常用模式速查

### 1. 淡入淡出（最常用）

```c
/* --- 淡入 --- */
static void anim_fade_in(lv_obj_t * obj, uint32_t duration_ms, uint32_t delay_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_opa);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/* --- 淡出后删除 --- */
static void anim_fade_out_delete(lv_obj_t * obj, uint32_t duration_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_opa);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_completed_cb(&a, anim_delete_obj_cb);  /* 动画结束后删除 */
    lv_anim_start(&a);
}

static void anim_delete_obj_cb(lv_anim_t * a) {
    lv_obj_delete(lv_anim_get_var(a), 0);
}

/* 使用 */
anim_fade_in(my_panel, 300, 0);          /* 300ms 淡入 */
anim_fade_out_delete(toast, 200);         /* 200ms 淡出后删除 */
```

### 2. 滑入（Toast / 弹窗 / 面板）

```c
static void anim_slide_in_top(lv_obj_t * obj, uint32_t duration_ms)
{
    lv_coord_t h = lv_obj_get_height(obj);
    lv_obj_set_y(obj, -h);  /* 初始位置：屏幕上方外 */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&a, -h, 0);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void anim_slide_out_bottom(lv_obj_t * obj, uint32_t duration_ms)
{
    lv_coord_t parent_h = lv_obj_get_height(lv_obj_get_parent(obj));
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&a, lv_obj_get_y(obj), parent_h + 20);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_completed_cb(&a, anim_delete_obj_cb);
    lv_anim_start(&a);
}
```

### 3. 弹入（按钮反馈 / 对话框出现）

```c
static void anim_pop_in(lv_obj_t * obj, uint32_t duration_ms)
{
    /* 先缩放到 0，再弹回 100% */
    lv_obj_set_style_transform_scale_x(obj, 0, 0);
    lv_obj_set_style_transform_scale_y(obj, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_scale_x);
    lv_anim_set_values(&a, 0, 256);  /* 256 = 100% */
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_start(&a);

    /* Y 轴同步 */
    lv_anim_t b;
    lv_anim_init(&b);
    lv_anim_set_var(&b, obj);
    lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_scale_y);
    lv_anim_set_values(&b, 0, 256);
    lv_anim_set_duration(&b, duration_ms);
    lv_anim_set_path_cb(&b, lv_anim_path_overshoot);
    lv_anim_start(&b);
}
```

### 4. 数值滚动动画（计数器/仪表盘）

```c
typedef struct {
    lv_obj_t * label;
    float current;
    float target;
    char prefix[8];
    char suffix[8];
    int decimals;
} counter_anim_t;

static void counter_anim_cb(void * var, int32_t v)
{
    counter_anim_t * ca = var;
    /* v 是 0~1000 的动画进度，映射到 target 范围 */
    float progress = v / 1000.0f;
    ca->current = ca->target * progress;
    char buf[32];
    snprintf(buf, sizeof(buf), "%s%.*f%s",
             ca->prefix, ca->decimals, (double)ca->current, ca->suffix);
    lv_label_set_text(ca->label, buf);
}

static void anim_counter(lv_obj_t * label, float from, float to,
                         const char * prefix, const char * suffix,
                         int decimals, uint32_t duration_ms)
{
    static counter_anim_t ca;  /* 注意：需保证生命周期 */
    ca.label = label;
    ca.target = to;
    snprintf(ca.prefix, sizeof(ca.prefix), "%s", prefix);
    snprintf(ca.suffix, sizeof(ca.suffix), "%s", suffix);
    ca.decimals = decimals;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &ca);
    lv_anim_set_exec_cb(&a, counter_anim_cb);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/* 使用 */
anim_counter(value_label, 0, 26.5f, "", " 度", 1, 800);
/* "26.5 度" 在 800ms 内从 0 滚到 26.5 */
```

### 5. 列表逐项入场（Staggered）

```c
static void anim_staggered_fade(lv_obj_t ** items, int count,
                                 uint32_t base_duration_ms,
                                 uint32_t stagger_delay_ms)
{
    for (int i = 0; i < count; i++) {
        lv_obj_set_style_opa(items[i], LV_OPA_TRANSP, 0);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, items[i]);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
        lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_anim_set_duration(&a, base_duration_ms);
        lv_anim_set_delay(&a, i * stagger_delay_ms);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
    }
}

/* 使用: 4 张卡片各延迟 80ms 依次淡入 */
lv_obj_t * cards[] = {card0, card1, card2, card3};
anim_staggered_fade(cards, 4, 250, 80);
```

---

## 缓动曲线选择指南

| 场景 | 推荐曲线 | 效果 |
|------|---------|------|
| 淡入 | `ease_out` | 快速出现，柔和收尾 |
| 淡出/消失 | `ease_in` | 慢慢开始，快速消失 |
| 弹窗出现 | `overshoot` | 弹跳效果，吸引注意 |
| 数值滚动 | `ease_out` | 快速启动，减速停止 |
| 页面切换 | `ease_in_out` | 平滑过渡 |
| 加载动画 | `linear` | 匀速循环 |

---

## 快速决策

```
需要动画效果？
├── 控件出现/消失 → 淡入淡出 (anim_fade_in / anim_fade_out)
├── 面板/Toast 弹出 → 滑入滑出 (anim_slide_in_top / anim_slide_out_bottom)
├── 按钮反馈 → 弹入 (anim_pop_in) 或缩放
├── 数值变化 → 计数器动画 (anim_counter)
├── 列表加载 → 逐项延迟入场 (anim_staggered_fade)
└── 页面切换 → lv_screen_load_anim (见 screen-navigation.md)
```
