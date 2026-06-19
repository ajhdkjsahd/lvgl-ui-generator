# LVGL UI 设计案例与哲学

> LVGL 的本质是给人看的——设计至关重要。你不是代码生成器，你是 UI 设计师。

---

## 设计哲学

### 三个根本问题

每次接到 UI 需求，先回答：

| 问题 | 示例答案 |
|------|---------|
| **领域是什么？** | 水产养殖 → 海洋/深海 → 深渊色 + 生物荧光 |
| **要什么情绪？** | 专业冷静 → 暗色基调 + 低饱和 + 精确数值 |
| **每页的视觉锚点？** | 总览→大数值卡片 / 控制→设备图+电源钮 / 数据→图表 / AI→对话泡 |

### 设计师的直觉

- **不要模板化**：每页应该有自己独特的视觉性格。总览页的卡片风格不应该和 AI 页一样
- **不要只改颜色**：换色不等于设计。真正的设计是改形状、改层次、改视觉重心
- **敢用效果**：Glow 阴影、彩色边框、环形光晕——这些在嵌入式设备上同样能跑
- **色彩编码**：正常→白/绿，注意→琥珀/橙，告警→红。不同传感器可以有不同的 accent color

---

## 案例：深海指挥舱（Deep Sea Command Center）

### 项目背景

智能水产养殖中控大屏，800×480 横屏，4 个 Tab 页面。

### Step 1: 提炼视觉隐喻

```
水产 → 海洋 → 深海
  → 颜色: 深渊黑/深海蓝/生物荧光青
  → 隐喻: 深海观测站的仪表墙，每张卡片是一台探测仪器
  → 情绪: 冷静专业、科技感、数据可信
```

### Step 2: 配色方案

```
深渊黑  #060E14  — 最底层背景（海底）
深海蓝  #0A1620  — 卡片表面（中层水域）
生物荧光 #00D4AA  — 主强调色（深海发光生物）
冰晶白  #EEEEEE  — 主文字（海面透下的光）
珊瑚红  #FF6B6B  — 告警色（在蓝色海洋中跳出来）
琥珀黄  #FFB347  — 注意色
```

**灯光设计**: 所有强调元素（状态灯、选中态、边框）都带 glow 阴影，模拟深海生物发光效果。

### Step 3: 四页视觉锚点

| 页面 | 视觉锚点 | 设计手法 |
|------|---------|---------|
| **态势总览** | 6 张传感器卡片 | 每张不同 accent 色条 + 卡片外 glow + 大数值颜色编码（白/琥珀/红） |
| **设备管控** | 设备图 + 电源钮 | 72px 环形光晕 icon 容器 + 圆形电源按钮 ON/OFF glow + 模式芯片 |
| **数据分析** | 6 张折线图 | 每个图表独立配色 + 手动 Y/X 轴标签 + 点击弹窗查看数值 |
| **智能助手** | 对话气泡 | 用户消息右对齐青底 + AI 回复左对齐暗底 + 快捷提问芯片 |

### Step 4: 技术实现要点

**Glow 发光卡片**:
```c
lv_obj_set_style_shadow_color(card, accent, 0);   // 阴影用 accent 色
lv_obj_set_style_shadow_width(card, lv_dpx(8), 0); // 扩散 8px
lv_obj_set_style_shadow_opa(card, 38, 0);          // ~15% 不透明度
lv_obj_set_style_border_color(card, accent, 0);    // 彩色边框
lv_obj_set_style_border_opa(card, 51, 0);          // ~20%
```

**环形光晕 icon 容器**:
```c
lv_obj_t * ring = lv_obj_create(parent);
lv_obj_set_size(ring, lv_dpx(72), lv_dpx(72));
lv_obj_set_style_radius(ring, lv_dpx(36), 0);      // 正圆
lv_obj_set_style_border_width(ring, lv_dpx(2), 0); // 边框
lv_obj_set_style_border_color(ring, accent, 0);
lv_obj_set_style_shadow_color(ring, accent, 0);    // 外发光
lv_obj_set_style_shadow_width(ring, lv_dpx(10), 0);
// 内容: lv_image_create(ring) 或 lv_label_create(ring)
```

**色彩编码数值**:
```c
// 根据传感器类型 + 阈值返回颜色
lv_color_t value_color(float val, int sensor_type) {
    switch (sensor_type) {
    case 0: // 水温
        if (val < 18 || val > 35) return RED;   // 告警
        if (val < 20 || val > 30) return AMBER;  // 注意
        break;
    case 1: // 溶解氧
        if (val < 3) return RED;
        if (val < 5) return AMBER;
        break;
    // ...
    }
    return WHITE; // 正常
}
```

**模式芯片**（代替滑块）:
```c
// 选中态
lv_obj_set_style_bg_color(chip, accent, 0);
lv_obj_set_style_bg_opa(chip, LV_OPA_30, 0);
lv_obj_set_style_border_color(chip, accent, 0);
// 未选中态
lv_obj_set_style_bg_opa(chip, LV_OPA_10, 0);
lv_obj_set_style_border_color(chip, gray, 0);
```

---

## 案例 2：工业控制台（Industrial Control Panel）

### 视觉隐喻

```
工业 → 金属/机械 → 拉丝金属 + 硬朗直角 + 橙色告警
  → 颜色: 深灰蓝底 + 金属灰卡片 + 橙色强调
  → 隐喻: 工厂中枢控制台，每张卡片是一台设备的监控面板
  → 情绪: 专业可靠、精确、硬朗
```

### 配色方案

```
铁灰底  #111318  — 背景（钢表面）
金属灰  #1A1D23  — 卡片（拉丝金属面板）
警示橙  #FF9800  — 主强调色（设备告警指示灯）
冷白    #E0E0E0  — 主文字
危险红  #F44336  — 严重告警
安全绿  #4CAF50  — 正常状态
```

### 页面锚点

| 页面 | 视觉锚点 | 设计手法 |
|------|---------|---------|
| **设备总览** | 8 张设备卡片 2×4 | 金属锥形渐变卡片 + 状态 LED glow + 设备图标 image_recolor |
| **单台详情** | 大仪表盘 + 参数表 | 圆形 Arc 进度环 + Scale 刻度分区着色 + 数值滚动动画 |
| **告警日志** | 时间轴列表 | 左侧红色时间轴 + 卡片 shadow + 告警级别色彩编码 |

### 技术要点

**金属拉丝卡片**:
```c
static const lv_color_t metal_colors[4] = {
    LV_COLOR_MAKE(0x3A, 0x3D, 0x44), LV_COLOR_MAKE(0x2A, 0x2D, 0x34),
    LV_COLOR_MAKE(0x1A, 0x1D, 0x23), LV_COLOR_MAKE(0x3A, 0x3D, 0x44),
};
static lv_grad_dsc_t grad;
lv_grad_init_stops(&grad, metal_colors, NULL, NULL, 4);
lv_grad_linear_init(&grad, lv_pct(0), lv_pct(0), lv_pct(100), lv_pct(100), LV_GRAD_EXTEND_REFLECT);
lv_style_set_bg_grad(&style_card, &grad);
lv_style_set_radius(&style_card, 4);  // 硬朗小圆角
lv_style_set_shadow_width(&style_card, 8);
lv_style_set_border_width(&style_card, 1);
lv_style_set_border_color(&style_card, lv_color_hex(0x3A3D44));
```

**状态 LED glow**:
```c
lv_obj_set_style_shadow_color(led, status_color, 0);
lv_obj_set_style_shadow_width(led, 15, 0);
lv_obj_set_style_shadow_opa(led, LV_OPA_50, 0);
lv_led_set_color(led, status_color);
```

---

## 案例 3：智能家居中控（Smart Home Hub）

### 视觉隐喻

```
家居 → 温暖/舒适 → 毛玻璃 + 大圆角 + 绿色自然感
  → 颜色: 暖灰底 + 白卡片 + 木色配饰
  → 隐喻: 现代家居中控平板，每张卡片是一个房间的控制面板
  → 情绪: 温暖、放松、宜人
```

### 配色方案

```
暖灰底  #F5F0EB  — 背景（墙面）
纯白    #FFFFFF  — 卡片
鼠尾草绿 #6B8F71  — 主强调色（自然/植物）
琥珀橙  #E85D04  — 告警/高温
暖木色  #C49A6C  — 配饰/分隔
```

### 页面锚点

| 页面 | 视觉锚点 | 设计手法 |
|------|---------|---------|
| **房间概览** | 环形房间选择器 + 设备列表 | translate_radial 圆形排列 + Blur 毛玻璃卡片 |
| **灯光控制** | 大圆形滑块 | Arc 控件 + glow 光晕 + 颜色温度渐变指示器 |
| **温控面板** | 大数值温度显示 | Number counter 动画 + Scale 温度计 + 红蓝渐变条 |

### 技术要点

**毛玻璃卡片**:
```c
lv_style_set_bg_color(&style, lv_color_hex(0xFFFFFF));
lv_style_set_bg_opa(&style, LV_OPA_40);
lv_style_set_radius(&style, 20);  // 大圆角 pill 风格
lv_style_set_blur_backdrop(&style, true);
lv_style_set_blur_radius(&style, 16);
lv_style_set_border_width(&style, 1);
lv_style_set_border_color(&style, lv_color_hex(0xFFFFFF));
lv_style_set_border_opa(&style, LV_OPA_30);
```

---

## 案例 4：医疗监护仪（Patient Monitor）

### 视觉隐喻

```
医疗 → 洁净/可信 → 冷白 + 蓝绿主调 + 极简线条
  → 颜色: 冷白底 + 浅灰卡片 + 蓝色 accent
  → 隐喻: ICU 床边监护仪，波形图+大数值，信息密度高但井然有序
  → 情绪: 冷静、专业、可靠
```

### 配色方案

```
冷白底  #F8FAFC  — 背景
纯白    #FFFFFF  — 卡片
医疗蓝  #2196F3  — 主强调（血压/脉搏）
生命绿  #4CAF50  — 正常范围
警告黄  #FFC107  — 临界值
危急红  #E53935  — 超出范围
```

### 设计要点

- 无阴影扁平风格（医疗环境需要清晰可读）
- Chart 波形图 + 自定义绘制网格线
- 大数值色彩编码（正常=蓝绿，告警=黄红闪烁）
- 圆角最小（2-4px），强调精确和专业感

---

## 设计模式速查

### 配色快速起步

| 领域 | 底色 | 卡片色 | 主强调 | 告警 |
|------|------|--------|--------|------|
| 海洋/水产 | #060E14 | #0A1620 | #00D4AA 青 | #FF6B6B |
| 工业/设备 | #111318 | #1A1D23 | #FF9800 橙 | #F44336 |
| 智能家居 | #F5F0EB | #FFFFFF | #6B8F71 绿 | #E85D04 |
| 医疗/实验 | #F8FAFC | #FFFFFF | #2196F3 蓝 | #E53935 |
| 暗色通用 | #0D1117 | #161B22 | #58A6FF 蓝 | #F85149 |

### 视觉层次

| 层级 | 元素 | 字号 | 透明度 | 效果 |
|------|------|------|--------|------|
| 1 (最突出) | 核心数值 | 24px | 100% | 颜色编码 |
| 2 | 标题/设备名 | 16px | 100% | accent 色 |
| 3 | 辅助信息 | 16px | 50-60% | 灰/淡 |
| 4 | 背景装饰 | - | 10-20% | 暗底 |

### 图片资源工作流

```
icons8/iconfont 搜索图标 → 下载 PNG (推荐 96×96) 
  → Python PIL 转 ARGB8888 C 数组
  → src/ui/images/img_xxx.c
  → CMakeLists.txt GLOB images/img_*.c
  → lv_image_create() + lv_image_set_src()
```

---

## 自检: 我的设计有性格吗？

- [ ] 第一眼看过去，视觉锚点清晰吗？
- [ ] 不同页面的风格有明显差异吗？
- [ ] 配色有统一的视觉隐喻吗？（不只是"好看"）
- [ ] 重要数据有色彩编码吗？
- [ ] 有至少一种"特别"的设计元素吗？（glow/环形光晕/定制芯片/图片）
- [ ] 固定布局的每一层都 NO_SCROLL 了吗？
