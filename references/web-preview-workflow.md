# Web 预览工作流 —— 设计确认后再写 LVGL 代码

> 核心理念：用户看到效果 → 确认满意 → 翻译为 LVGL C 代码。避免 AI 盲目生成不符合用户预期的 UI。

---

## 完整流程

```
用户描述需求
  → AI 生成单文件 HTML（CSS 模拟 LVGL 控件外观）
  → 启动本地 HTTP 服务器（python -m http.server 8765）
  → Playwright 浏览器截图
  → 用户看到截图，反馈修改意见
  → AI 修改 HTML → 刷新截图（5 秒迭代）
  → 用户确认："就这样！"
  → AI 翻译为 LVGL v9 C 代码
  → 编译部署
```

---

## 一、HTML 预览生成规范

### 1.1 必须模拟的 LVGL 视觉特性

| LVGL 概念 | CSS 模拟方式 |
|----------|-------------|
| 圆屏（手表） | `border-radius:50%` + `overflow:hidden` + SVG clip-path 备用 |
| 方屏（嵌入式） | 固定宽高容器，模拟目标分辨率 |
| 背景渐变（线性） | `background: linear-gradient(...)` |
| 背景渐变（径向） | `background: radial-gradient(...)` |
| 背景渐变（锥形） | `conic-gradient(...)` + `-webkit-mask` 模拟金属圈 |
| Box Shadow | `box-shadow` |
| Drop Shadow | `filter: drop-shadow(...)` |
| Glow 发光 | `box-shadow: 0 0 Npx accent` |
| Blur 毛玻璃 | `backdrop-filter: blur(Npx)` |
| 不透明度 | `opacity` / `rgba()` |
| 圆角 | `border-radius` |
| 边框 | `border` |
| Flex 布局 | `display: flex` + `flex-direction/align/justify` |
| Grid 布局 | `display: grid` + `grid-template-columns/rows` |
| 旋转（指针） | `transform: rotate(Ndeg)` |
| 缩放 | `transform: scale(N)` |
| 倾斜 | `transform: skew(Ndeg)` |
| 混合模式 | `mix-blend-mode` |
| 动画 | `@keyframes` + `animation` |
| 字体 | Google Fonts 或系统字体近似 |

### 1.2 HTML 文件结构模板

```html
<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>LVGL UI 预览 — [页面名称]</title>
<style>
  /* ===== 全局 ===== */
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    background:#1a1a2e;
    display:flex; justify-content:center; align-items:center;
    min-height:100vh;
    font-family:'Segoe UI','PingFang SC','Microsoft YaHei',sans-serif;
  }

  /* ===== 目标设备模拟 ===== */
  .device-frame {
    /* 圆屏手表 */
    width:360px; height:360px;
    border-radius:50%;
    overflow:hidden;
    position:relative;
    /* 方屏嵌入式 */
    /* width:800px; height:480px; */
  }

  /* ===== 页面内容 ===== */
  /* ... 具体样式 ... */
</style>
</head>
<body>
<div class="device-frame">
  <!-- 页面内容 -->
</div>
</body>
</html>
```

### 1.3 预览步骤（Claude 执行）

```python
# 1. 写入 HTML 文件
Write(preview_xxx.html)

# 2. 启动 HTTP 服务器（如果还没启动）
Bash("cd project_dir && python -m http.server 8765 &")

# 3. Playwright 打开页面
browser_navigate("http://localhost:8765/preview_xxx.html")

# 4. 截图
browser_take_screenshot(filename="preview_xxx.png")

# 5. 读取截图确认效果
Read("preview_xxx.png")

# 6. 根据用户反馈修改 HTML，重复 3-5
```

---

## 二、CSS → LVGL 翻译表

### 2.1 布局翻译

| CSS | LVGL v9 |
|-----|---------|
| `display:flex; flex-direction:row` | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW)` |
| `display:flex; flex-direction:column` | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN)` |
| `display:flex; flex-wrap:wrap` | `LV_FLEX_FLOW_ROW_WRAP` |
| `justify-content: center` | `LV_FLEX_ALIGN_CENTER` (main_place) |
| `align-items: center` | `LV_FLEX_ALIGN_CENTER` (cross_place) |
| `justify-content: space-between` | `LV_FLEX_ALIGN_SPACE_BETWEEN` |
| `justify-content: space-evenly` | `LV_FLEX_ALIGN_SPACE_EVENLY` |
| `display:grid; grid-template-columns` | `lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc)` |
| `grid-column: 1 / span 2` | `lv_obj_set_grid_cell(obj, ..., col_pos, col_span, ...)` |
| `position:absolute; top:50%; left:50%` | `lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0)` |
| `position:absolute; bottom:20px; right:20px` | `lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, -20, -20)` |
| `flex-grow: 1` | `lv_obj_set_flex_grow(obj, 1)` |
| `gap: 10px` | `lv_obj_set_style_pad_row/column(cont, 10, 0)` |
| `padding: 16px` | `lv_obj_set_style_pad_all(obj, 16, 0)` |

### 2.2 视觉特效翻译

| CSS | LVGL v9 |
|-----|---------|
| `background: #1a1a2e` | `lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a2e), 0)` |
| `background: linear-gradient(180deg, #6366f1, #ec4899)` | `bg_color=#6366f1` + `bg_grad_color=#ec4899` + `bg_grad_dir=LV_GRAD_DIR_VER` |
| `background: radial-gradient(circle at 35% 35%, #1e2a4a, #0b0b2b)` | `lv_grad_radial_init()` + stops + focal point |
| `background: conic-gradient(...)` | `lv_grad_conical_init()` + multi stops + `LV_GRAD_EXTEND_REFLECT` |
| `box-shadow: 0 8px 30px rgba(0,0,0,0.3)` | `shadow_width=30` + `shadow_offset_y=8` + `shadow_opa=77` |
| `box-shadow: 0 0 20px accent` | `shadow_width=20` + `shadow_color=accent` + `shadow_opa=40` |
| `filter: drop-shadow(0 4px 16px rgba(0,0,0,0.3))` | `drop_shadow_radius=16` + `drop_shadow_offset_y=4` + `drop_shadow_opa=77` |
| `backdrop-filter: blur(18px)` | `blur_backdrop=true` + `blur_radius=18` |
| `opacity: 0.5` | `opa=LV_OPA_50` |
| `background: rgba(255,255,255,0.4)` | `bg_color=white` + `bg_opa=LV_OPA_40` |
| `border-radius: 12px` | `radius=12` |
| `border-radius: 50%` | `radius=LV_RADIUS_CIRCLE` |
| `border: 2px solid #333` | `border_width=2` + `border_color=#333` |
| `border: 2px solid; border-color: accent transparent transparent` | `border_side=LV_BORDER_SIDE_TOP` + `border_color=accent` |
| `transform: rotate(45deg)` | `transform_rotation=450` (0.1度单位) |
| `transform: scale(1.2)` | `transform_scale_x=307` `transform_scale_y=307` (256*1.2) |
| `mix-blend-mode: screen` | `blend_mode=LV_BLEND_MODE_ADDITIVE` |
| `mix-blend-mode: multiply` | `blend_mode=LV_BLEND_MODE_MULTIPLY` |
| `text-shadow: 0 0 20px accent` | `text_outline_stroke_color=accent` + `text_outline_stroke_width` |
| `color: #e8f0ff` | `text_color=lv_color_hex(0xe8f0ff)` |
| `font-size: 24px` | `text_font=&lv_font_montserrat_24` (或 FreeType 24px) |
| `letter-spacing: 4px` | `text_letter_space=4` |
| `font-weight: 200` | 选对应字重的字体变量 |

### 2.3 颜色转换

```c
// CSS #RRGGBB → LVGL lv_color_hex()
// 注意：LVGL 内部可能是 RGB565，颜色有轻微精度损失
// 暗色系损失少，亮色系渐变可能有条带
lv_color_hex(0xRRGGBB)  // 直接使用
lv_color_make(R, G, B)   // RGB 分量 0-255
```

### 2.4 不能翻译的 CSS 技巧

| CSS | 替代方案 |
|-----|---------|
| `::before / ::after` 伪元素 | 创建额外的 lv_obj_t |
| `@keyframes` 复杂动画 | 用 `lv_anim_t` 手动编排 |
| `clip-path` 不规则裁剪 | `bitmap_mask_src` (A8 位图蒙版) |
| `text-overflow: ellipsis` | `lv_label_set_long_mode(LV_LABEL_LONG_DOTS)` |
| `overflow: scroll` | `lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE)` |

---

## 三、常见设备模板

### 3.1 圆屏手表（360×360）— 完整模板

```html
<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>LVGL 手表预览</title>
<style>
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    background:#1a1a2e;
    display:flex; justify-content:center; align-items:center;
    min-height:100vh;
    font-family:'Segoe UI','PingFang SC','Microsoft YaHei',sans-serif;
  }
  .watch-frame {
    /* 手表外壳 */
    width:380px; height:380px; border-radius:50%;
    background: #1a1a2e;
    box-shadow: 0 0 40px rgba(0,0,0,0.5), 0 0 80px rgba(0,100,200,0.1);
    display:flex; justify-content:center; align-items:center;
  }
  .watch-face {
    width:360px; height:360px; border-radius:50%;
    overflow:hidden; position:relative;
    /* 深空背景 */
    background: radial-gradient(circle at 35% 35%, #1e2a4a, #0b122e 60%, #060a1a);
    box-shadow: inset 0 0 30px rgba(0,0,0,0.5);
  }
  /* 金属表圈 */
  .bezel {
    position:absolute; inset:6px; border-radius:50%;
    border:3px solid transparent;
    background: conic-gradient(from 0deg,
      #3a4a6c, #5a6a8c, #3a4a6c, #2a3a5c, #4a5a7c, #3a4a6c,
      #2a3a5c, #4a5a7c, #3a4a6c, #5a6a8c, #3a4a6c, #2a3a5c
    ) border-box;
    -webkit-mask: radial-gradient(farthest-side, transparent calc(100% - 3px), #000 calc(100% - 3px));
    mask: radial-gradient(farthest-side, transparent calc(100% - 3px), #000 calc(100% - 3px));
  }
  /* 刻度 tick */
  .tick { position:absolute; background:#8899bb; }
  .tick.major { width:2px; height:12px; background:#bbccdd; }
  .tick.minor { width:1px; height:6px; background:#556688; }
  /* 指针 */
  .hand {
    position:absolute; bottom:50%; left:50%;
    transform-origin: bottom center;
    border-radius:2px;
  }
  .hand-hour {
    width:5px; height:68px; margin-left:-2.5px;
    background: linear-gradient(to top, #c9a96e 60%, #e8d5a3);
    box-shadow: 0 0 12px rgba(201,169,110,0.3);
  }
  .hand-minute {
    width:3px; height:96px; margin-left:-1.5px;
    background: linear-gradient(to top, #a0b0c0 70%, #d0e0f0);
    box-shadow: 0 0 8px rgba(160,176,192,0.25);
  }
  .hand-second {
    width:1.5px; height:108px; margin-left:-0.75px;
    background: #ff4050;
    box-shadow: 0 0 6px rgba(255,64,80,0.4);
  }
  .center-cap {
    position:absolute; top:50%; left:50%;
    width:14px; height:14px; margin:-7px;
    border-radius:50%;
    background: radial-gradient(circle at 35% 35%, #e8d5a3, #8b734b);
    box-shadow: 0 2px 8px rgba(0,0,0,0.5);
  }
  /* 数字化信息 */
  .info-digital {
    position:absolute; text-align:center; width:100%;
    color:rgba(200,220,255,0.5);
    font-size:12px; letter-spacing:2px;
  }
  .brand {
    position:absolute; top:68%; left:50%; transform:translate(-50%,0);
    font-size:10px; color:rgba(200,220,255,0.25); letter-spacing:4px;
    text-transform:uppercase;
  }
  /* 星点 */
  .star { position:absolute; background:white; border-radius:50%; }
  @keyframes twinkle { 0%,100%{opacity:0.2;} 50%{opacity:1;} }
</style>
</head>
<body>
<div class="watch-frame">
  <div class="watch-face" id="watchFace">
    <div class="bezel"></div>
    <!-- 星点（JS 动态生成 60-80 颗） -->
    <div id="stars"></div>
    <!-- 刻度（JS 动态生成 60 个） -->
    <div id="ticks"></div>
    <!-- 时间 -->
    <div class="info-digital" style="top:28%; font-size:52px; font-weight:200;
         color:#e8f0ff; letter-spacing:4px;
         text-shadow: 0 0 20px rgba(100,180,255,0.3);">10:42</div>
    <!-- 日期 -->
    <div class="info-digital" style="top:44%;">6月19日 周五</div>
    <!-- 步数 -->
    <div class="info-digital" style="top:76%; color:rgba(0,212,170,0.6);">🚶 8,247 步</div>
    <!-- 品牌 -->
    <div class="brand">AURORA</div>
    <!-- 指针 -->
    <div class="hand hand-hour" style="transform:rotate(320deg)"></div>
    <div class="hand hand-minute" style="transform:rotate(45deg)"></div>
    <div class="hand hand-second" style="transform:rotate(180deg)"></div>
    <div class="center-cap"></div>
  </div>
</div>
<script>
// 生成星点
(function() {
  var face = document.getElementById('watchFace');
  var w = 360, r = w/2 - 8;
  var html = '';
  for(var i = 0; i < 70; i++) {
    var angle = Math.random() * 360;
    var dist = Math.random() * r * 0.7 + r * 0.3;
    var x = r + Math.cos(angle * Math.PI/180) * dist;
    var y = r + Math.sin(angle * Math.PI/180) * dist;
    var size = Math.random() * 1.5 + 0.5;
    var delay = Math.random() * 3;
    html += '<div class="star" style="width:'+size+'px;height:'+size+'px;'
         + 'left:'+x+'px;top:'+y+'px;'
         + 'animation:twinkle '+ (2+Math.random()*3) +'s infinite '
         + delay+'s"></div>';
  }
  document.getElementById('stars').innerHTML = html;

  // 生成刻度
  html = '';
  for(var i = 0; i < 60; i++) {
    var angle = i * 6 - 90;
    var rad = angle * Math.PI/180;
    var x1 = r + Math.cos(rad) * (r - 14);
    var y1 = r + Math.sin(rad) * (r - 14);
    var cls = (i % 5 === 0) ? 'tick major' : 'tick minor';
    html += '<div class="'+cls+'" style="left:'+(x1-0.5)+'px;top:'+y1+'px;'
         + 'transform:rotate('+angle+'deg)"></div>';
  }
  document.getElementById('ticks').innerHTML = html;
})();
</script>
</body>
</html>
```

### 3.2 嵌入式方屏（800×480）

```css
.embedded-screen {
  width:800px; height:480px; overflow:hidden;
  background: var(--bg);
  /* 通常使用 Flex 或 Grid 内部布局 */
}
```

### 3.3 方屏仪表盘（480×480）

```css
.dashboard {
  width:480px; height:480px; overflow:hidden;
  background: var(--bg);
  display:grid;
  grid-template-columns: 1fr 1fr;
  grid-template-rows: 1fr 1fr;
  gap: 12px; padding: 16px;
}
```

---

## 六、HTML 预览确认后（最后一步）

**HTML 预览确认 → 翻译为 LVGL C 代码 → 用 SDL 验证真机效果。**

Web 预览用于设计阶段的快速迭代，但 LVGL 真机渲染可能与 CSS 有细微差异（字体、色深 RGB565、动画帧率）。翻译完成后，建议用 `references/preview-workflow.md` 的 SDL PC 预览工程编译运行，确认真机效果。

```
HTML预览（设计确认用，5秒迭代）
  → 确认 → 翻译C代码
    → SDL PC预览（真机验证用，编译运行看实际LVGL渲染）
      → 真机烧录
```

---

## 四、迭代速度对比

| 操作 | HTML 预览 | 直接写 C 代码 |
|------|----------|-------------|
| 修改配色 | 5 秒 | 改代码→编译→部署 5-10 分钟 |
| 调整布局 | 10 秒 | 5-10 分钟 |
| 换字体/字号 | 5 秒 | 可能需要重新生成字体 |
| 看圆屏效果 | 即时 | 必须烧录到真机 |
| 用户确认设计 | 2-3 轮 HTML 迭代 | 反复编译部署 |
| 最终 C 代码 | 一次生成 | 反复修改 |
