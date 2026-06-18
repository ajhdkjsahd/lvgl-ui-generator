# 图标显示规范

LVGL 内置符号基于 Font Awesome 5，通过 `LV_SYMBOL_*` 宏使用。源码定义在 `lvgl/include/lvgl/font/lv_symbol_def.h`。

---

## 核心规则

**`LV_SYMBOL_*` 的码位在 Montserrat 字体的私有区，和中文字体不互通。同一个 label 不能同时渲染中文和图标 — 必须拆成两个 label，各用各的字体：**

```c
// ✅ 正确：图标 label（montserrat） + 中文 label（自定义字体），用 row 包裹
lv_obj_t * row = create_inline_row(parent, 776, 36);

lv_obj_t * icon = lv_label_create(row);
lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
lv_label_set_text(icon, LV_SYMBOL_HOME);

lv_obj_t * text = lv_label_create(row);
lv_obj_set_style_text_font(text, &lv_font_smart_home_24, 0);
lv_obj_set_style_pad_left(text, 8, 0);   // 间距用 padding，不写空格
lv_label_set_text(text, "智能家居控制中心");
```

```c
// ❌ 错误：图标和中文在同一个 label
lv_label_set_text(label, LV_SYMBOL_HOME " 智能家居");
// → 图标或中文之一会显示为方框
```

---

## 图标选择方法论

**必须按英文关键词在符号名中搜索匹配，不能凭感觉乱选。**

```
1. 先将中文含义翻译成英文关键词
   例如："温度" → temperature, temp, heat, warm, bolt, energy

2. 在下方符号表中按关键词搜索最匹配的符号名
   例如：搜 "tint" → LV_SYMBOL_TINT（水滴💧）→ 匹配湿度 ✅

3. 无直接匹配时，选语义最接近的（功能类比 > 形状类比）
   例如：风速 → 无 WIND → 找 AIR/FLOW → 无 → 选 REFRESH（循环箭头≈气流循环）
```

**优先级：语义关键词匹配 > 功能类比 > 形状类比 >>> 随机挑选 ❌**

---

## 完整 LV_SYMBOL 列表（62 个，按 Font Awesome 名称分组）

### 导航与方向
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_HOME` | home | 🏠 | 首页、主页、回家 |
| `LV_SYMBOL_LEFT` | chevron-left | ◀ | 左、返回、上一页 |
| `LV_SYMBOL_RIGHT` | chevron-right | ▶ | 右、前进、下一页 |
| `LV_SYMBOL_UP` | chevron-up | ▲ | 上、展开 |
| `LV_SYMBOL_DOWN` | chevron-down | ▼ | 下、收起、下拉 |
| `LV_SYMBOL_GPS` | location-arrow | 📍 | 定位、导航、位置 |
| `LV_SYMBOL_DIRECTORY` | folder | 📁 | 文件夹、目录 |

### 媒体控制
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_PLAY` | play | ▶ | 播放、开始、运行 |
| `LV_SYMBOL_PAUSE` | pause | ⏸ | 暂停 |
| `LV_SYMBOL_STOP` | stop | ⏹ | 停止 |
| `LV_SYMBOL_PREV` | backward | ⏮ | 上一曲、快退 |
| `LV_SYMBOL_NEXT` | forward | ⏭ | 下一曲、快进 |
| `LV_SYMBOL_EJECT` | eject | ⏏ | 弹出 |
| `LV_SYMBOL_AUDIO` | music | ♪ | 音频、音乐、声音 |
| `LV_SYMBOL_VIDEO` | film | 🎬 | 视频、影片 |
| `LV_SYMBOL_VOLUME_MID` | volume-down | 🔉 | 音量中 |
| `LV_SYMBOL_VOLUME_MAX` | volume-up | 🔊 | 音量大、最大音量 |
| `LV_SYMBOL_MUTE` | volume-off | 🔇 | 静音、无声 |
| `LV_SYMBOL_SHUFFLE` | random | 🔀 | 随机、乱序、交叉 |

### 操作与状态
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_OK` | check | ✓ | 确认、通过、成功 |
| `LV_SYMBOL_CLOSE` | times | ✕ | 关闭、取消、删除 |
| `LV_SYMBOL_POWER` | power-off | ⏻ | 开关、电源、关机 |
| `LV_SYMBOL_PLUS` | plus | ＋ | 加、新增、放大 |
| `LV_SYMBOL_MINUS` | minus | － | 减、缩小 |
| `LV_SYMBOL_REFRESH` | sync | 🔄 | 刷新、同步、循环、空气流动 |
| `LV_SYMBOL_LOOP` | retweet | 🔁 | 循环、重复、环路 |
| `LV_SYMBOL_UPLOAD` | upload | ⬆ | 上传、发送 |
| `LV_SYMBOL_DOWNLOAD` | download | ⬇ | 下载、接收 |
| `LV_SYMBOL_SAVE` | floppy | 💾 | 保存、存储 |
| `LV_SYMBOL_EDIT` | pencil | ✏ | 编辑、修改 |
| `LV_SYMBOL_TRASH` | trash | 🗑 | 删除、垃圾桶 |
| `LV_SYMBOL_CUT` | cut | ✂ | 剪切 |
| `LV_SYMBOL_COPY` | copy | 📋 | 复制 |
| `LV_SYMBOL_PASTE` | paste | 📋 | 粘贴 |
| `LV_SYMBOL_BACKSPACE` | backspace | ⌫ | 退格、删除 |
| `LV_SYMBOL_NEW_LINE` | level-down-alt | ↵ | 换行、回车 |
| `LV_SYMBOL_SEARCH` | search | 🔍 | 搜索、查找 |

### 连接与设备
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_WIFI` | wifi | 📶 | WiFi、无线、网络 |
| `LV_SYMBOL_BLUETOOTH` | bluetooth | 🛜 | 蓝牙 |
| `LV_SYMBOL_USB` | usb | 🔌 | USB、连接 |
| `LV_SYMBOL_SD_CARD` | sd-card | 💳 | SD卡、存储卡 |
| `LV_SYMBOL_DRIVE` | hdd | 💾 | 硬盘、存储 |
| `LV_SYMBOL_KEYBOARD` | keyboard | ⌨ | 键盘、输入 |

### 电池系列
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_BATTERY_FULL` | battery-full | 🔋 | 满电、100% |
| `LV_SYMBOL_BATTERY_3` | battery-three-quarters | 🔋 | 75% |
| `LV_SYMBOL_BATTERY_2` | battery-half | 🔋 | 50% |
| `LV_SYMBOL_BATTERY_1` | battery-quarter | 🔋 | 25% |
| `LV_SYMBOL_BATTERY_EMPTY` | battery-empty | 🪫 | 空电、0% |
| `LV_SYMBOL_CHARGE` | bolt | ⚡ | 闪电、充电、能量、温度/热量 |

### 自然与感官
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_TINT` | tint | 💧 | **水滴**、湿度、水分、颜色 |
| `LV_SYMBOL_EYE_OPEN` | eye | 👁 | 可见、显示、查看 |
| `LV_SYMBOL_EYE_CLOSE` | eye-slash | 👁‍🗨 | 隐藏、不可见 |
| `LV_SYMBOL_BELL` | bell | 🔔 | 通知、提醒、铃声 |
| `LV_SYMBOL_WARNING` | exclamation-triangle | ⚠ | 警告、注意、危险 |
| `LV_SYMBOL_IMAGE` | image | 🖼 | 图片、图像、照片、场景 |
| `LV_SYMBOL_FILE` | file | 📄 | 文件、文档 |
| `LV_SYMBOL_STAR` | star | ⭐ | 收藏、星标、亮度 |

### 通信
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_CALL` | phone | 📞 | 电话、通话 |
| `LV_SYMBOL_ENVELOPE` | envelope | ✉ | 邮件、消息 |

### 其他
| 符号 | FA 名称 | 视觉 | 语义关键词 |
|------|---------|------|-----------|
| `LV_SYMBOL_SETTINGS` | cog | ⚙ | 设置、齿轮、**调节/仪表/气压** |
| `LV_SYMBOL_LIST` | list | 📋 | 列表 |
| `LV_SYMBOL_BARS` | bars | ☰ | 菜单、汉堡菜单 |
| `LV_SYMBOL_HEART` | heart | ❤ | 喜欢、健康 |
| `LV_SYMBOL_USER` | user | 👤 | 用户、我的 |
| `LV_SYMBOL_BULLET` | circle | ● | 圆点 |
| `LV_SYMBOL_DUMMY` | (placeholder) | — | 占位符 |

---

## Dropdown 箭头修复

Dropdown 设置中文字体后箭头会变方框，必须额外设置 `LV_PART_INDICATOR`：

```c
lv_obj_set_style_text_font(dd, &lv_font_app_16, LV_PART_MAIN);
lv_obj_set_style_text_font(dd, &lv_font_montserrat_16, LV_PART_INDICATOR);
```

---

## 两级图标方案

### Tier 1: LV_SYMBOL（默认）
图标 ≤10 个且 LV_SYMBOL（62 个）能覆盖 → 零成本直接使用。

### Tier 2: FA6 合并字体（需要语义精确时）
图标需求复杂（气象/医疗/工业）→ 将 FA6 Free Solid 与中文字体合并为单一字体。

**流程**：下载 FA6 → 查码位 → `lv_font_conv` 多字体合并 → 图标+中文单 label 渲染。
详见 `references/font-pipeline.md` 的「合并字体」章节。

### 方案对比

| 方案 | 图标数 | 图标+中文 | 集成成本 |
|------|--------|----------|----------|
| LV_SYMBOL | 62 | 分两个 label | 零 |
| FA6 合并字体 | 2000+ | **一个 label** | 下载字库 + 生成 |

> **原则：图标语义精确性优先。** 可用的 LV_SYMBOL 达不到语义要求时，升级到 FA6 合并字体。
