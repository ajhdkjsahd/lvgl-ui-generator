# 屏幕导航框架

## 导航模式选择

| 模式 | 适用场景 | 内存 | 复杂度 |
|------|----------|------|--------|
| **屏幕栈** (push/pop) | 多级菜单、详情页、设置子页 | 中 | 中 |
| **TabView** | 底部 2-5 Tab 平铺（首页/设备/场景/设置） | 低 | 低 |
| **预创建全屏** | 页面数 ≤ 5，内存充裕 | 高 | 低 |
| **延迟创建** | 页面多或内存紧张 | 低 | 中 |

推荐组合：**TabView 做主框架 + 栈式导航做子页跳转**。

---

## 1. 屏幕栈（Stack Navigation）

### 基础实现

```c
#define SCREEN_STACK_MAX 8

static lv_obj_t * screen_stack[SCREEN_STACK_MAX];
static int screen_stack_top = -1;

static void nav_push(lv_obj_t * screen, lv_screen_load_anim_t anim) {
    if (screen_stack_top < SCREEN_STACK_MAX - 1) {
        screen_stack[++screen_stack_top] = lv_screen_active();
    }
    lv_screen_load_anim(screen, anim, 300, 0);
}

static void nav_pop(lv_screen_load_anim_t anim) {
    if (screen_stack_top >= 0) {
        lv_obj_t * prev = screen_stack[--screen_stack_top];  // 注意：先取再减
        lv_screen_load_anim(prev, anim, 300, 0);
    }
}

static void nav_replace(lv_obj_t * screen, lv_screen_load_anim_t anim) {
    lv_screen_load_anim(screen, anim, 300, 0);
    // 不压栈，当前页被替换（如登录后跳主页）
}
```

### 使用方式

```c
// 页面创建函数中，返回按钮回调
static void on_back_click(lv_event_t * e) {
    nav_pop(LV_SCREEN_LOAD_ANIM_OVER_RIGHT);
}

// 列表项点击 → 进入详情
static void on_item_click(lv_event_t * e) {
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    lv_obj_t * detail = detail_page_create(idx);
    nav_push(detail, LV_SCREEN_LOAD_ANIM_OVER_LEFT);
}
```

---

## 2. TabView 导航

```c
lv_obj_t * create_main_ui(void) {
    lv_obj_t * tv = lv_tabview_create(lv_screen_active());

    // 底部 Tab 栏
    lv_tabview_set_tab_bar_position(tv, LV_DIR_BOTTOM);
    lv_obj_set_size(tv, 800, 480);

    lv_obj_t * tab_home    = lv_tabview_add_tab(tv, LV_SYMBOL_HOME " 首页");
    lv_obj_t * tab_device  = lv_tabview_add_tab(tv, LV_SYMBOL_SETTINGS " 设备");
    lv_obj_t * tab_scene   = lv_tabview_add_tab(tv, LV_SYMBOL_IMAGE " 场景");
    lv_obj_t * tab_profile = lv_tabview_add_tab(tv, LV_SYMBOL_USER " 我的");

    // 在每个 tab 内构建页面内容
    build_home_tab(tab_home);
    build_device_tab(tab_device);
    build_scene_tab(tab_scene);
    build_profile_tab(tab_profile);

    return tv;
}
```

**Tab 栏位置**：`LV_DIR_TOP` / `LV_DIR_BOTTOM` / `LV_DIR_LEFT` / `LV_DIR_RIGHT`

### Tab 内嵌栈式导航

当 Tab 内部的页面需要更深层跳转时，可以**替换该 Tab 内容**：

```c
static void on_device_item_click(lv_event_t * e) {
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);

    // 获取当前 Tab 页容器
    lv_obj_t * tab = lv_obj_get_child(lv_tabview_get_content(tv),
                                       lv_tabview_get_tab_active(tv));

    lv_obj_clean(tab);  // 清空当前内容
    build_device_detail(tab, idx);  // 构建详情
}
```

---

## 3. 预创建 vs 延迟创建

### 决策表

| 条件 | 策略 |
|------|------|
| 页面数 ≤ 5，每页简单 | 预创建全部 |
| 页面数 > 5 或有重页面 | 延迟创建 |
| 需要快速切换动画 | 预创建 |
| 内存 < 512KB 空闲 | 延迟创建 |

### 预创建模式

```c
// main.c
static lv_obj_t * home_screen;
static lv_obj_t * settings_screen;
static lv_obj_t * device_screen;

void app_init(void) {
    home_screen = home_page_create();
    settings_screen = settings_page_create();
    device_screen = device_page_create();
    lv_screen_load(home_screen);
}
```

### 延迟创建 + 缓存

```c
static lv_obj_t * g_page_cache[PAGE_COUNT] = {NULL};

lv_obj_t * get_page(int page_id) {
    if (g_page_cache[page_id]) return g_page_cache[page_id];

    switch (page_id) {
        case PAGE_HOME:    g_page_cache[page_id] = home_page_create();    break;
        case PAGE_DEVICE:  g_page_cache[page_id] = device_page_create();  break;
    }
    return g_page_cache[page_id];
}
```

---

## 4. 屏幕生命周期事件

```c
// 在页面创建函数中注册
static void on_screen_loaded(lv_event_t * e) {
    // 屏幕加载完成 — 刷新动态数据
    lv_obj_t * screen = lv_event_get_target(e);
    page_ctx_t * ctx = lv_event_get_user_data(e);
    update_sensor_values(ctx);
}

static void on_screen_unloaded(lv_event_t * e) {
    // 屏幕即将离开 — 保存状态、停止定时器
    page_ctx_t * ctx = lv_event_get_user_data(e);
    if (ctx->update_timer) {
        lv_timer_delete(ctx->update_timer);
        ctx->update_timer = NULL;
    }
}

lv_obj_t * my_page_create(void) {
    page_ctx_t * ctx = lv_malloc(sizeof(page_ctx_t));
    lv_obj_t * screen = lv_obj_create(NULL);

    lv_obj_add_event_cb(screen, on_screen_loaded, LV_EVENT_SCREEN_LOADED, ctx);
    lv_obj_add_event_cb(screen, on_screen_unloaded, LV_EVENT_SCREEN_UNLOADED, ctx);
    lv_obj_add_event_cb(screen, on_screen_delete, LV_EVENT_DELETE, ctx);  // 释放 ctx

    return screen;
}
```

### 常用生命周期事件

| 事件 | 时机 | 典型用途 |
|------|------|----------|
| `LV_EVENT_SCREEN_LOADED` | 屏幕切换动画完成后 | 刷新数据、启动定时器 |
| `LV_EVENT_SCREEN_UNLOADED` | 屏幕离开时 | 停止定时器、保存状态 |
| `LV_EVENT_SCREEN_LOAD_START` | 屏幕切换动画开始前 | 预加载资源 |
| `LV_EVENT_DELETE` | 屏幕被删除时 | 释放 ctx 内存 |

---

## 5. 切换动画

```c
// 前进（深入）：从左滑入
lv_screen_load_anim(new_screen, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 300, 0);

// 返回：从右滑入
lv_screen_load_anim(prev_screen, LV_SCREEN_LOAD_ANIM_OVER_RIGHT, 300, 0);

// 模态弹窗：淡入
lv_screen_load_anim(modal, LV_SCREEN_LOAD_ANIM_FADE_ON, 200, 0);
```

| 动画类型 | 语义 | 适用 |
|----------|------|------|
| `OVER_LEFT` | 新页从左滑入 | 前进/深入 |
| `OVER_RIGHT` | 新页从右滑入 | 返回/退出 |
| `OVER_TOP` | 从顶部滑入 | 通知/下拉 |
| `OVER_BOTTOM` | 从底部滑入 | 弹出面板 |
| `FADE_ON` / `FADE_OFF` | 淡入淡出 | 模态/弹窗 |
| `MOVE_LEFT` / `MOVE_RIGHT` | 平移（当前页也移动） | 同级切换 |
| `NONE` | 无动画 | 初始化/替换 |

---

## 6. 底部导航栏模式

不使用 TabView 时，可以用自定义底部栏 + 手动屏幕切换：

```c
typedef struct {
    lv_obj_t * screens[4];
    lv_obj_t * nav_btns[4];
    int active_idx;
} bottom_nav_t;

static void on_nav_btn_click(lv_event_t * e) {
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    bottom_nav_t * nav = /* 获取导航上下文 */;

    // 高亮当前按钮
    for (int i = 0; i < 4; i++) {
        lv_obj_remove_state(nav->nav_btns[i], LV_STATE_CHECKED);
    }
    lv_obj_add_state(nav->nav_btns[idx], LV_STATE_CHECKED);

    // 切换屏幕
    lv_screen_load(nav->screens[idx]);
    nav->active_idx = idx;
}
```
