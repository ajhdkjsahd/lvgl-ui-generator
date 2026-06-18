# 中文字体生成管线

## 字体方案对比

LVGL v9 有四种中文方案，按资源约束选择：

| 方案 | 适用场景 | ROM 开销 | RAM 开销 | 优点 | 缺点 |
|------|----------|----------|----------|------|------|
| **lv_font_conv** (位图) | 中文字 < 500 个，固定 2-3 个字号 | ~200KB-1MB | 0 | 零运行时开销，渲染快 | 需预生成，加字要重跑 |
| **lv_binfont** (预编译位图) | 同 lv_font_conv，但需文件系统 | ~200KB-1MB（文件） | 0 | 加载快，可动态切换 | 需要文件系统支持 |
| **Tiny TTF** | 板子有存储，中文量大 | .ttf 文件 ~5-15MB | ~50KB 缓存 | 不需要预生成，任意字号 | 首次渲染有延迟，需要 .ttf 部署 |
| **FreeType** | 需要高质量渲染 | libfreetype ~1MB + .ttf | ~100KB | 最佳渲染质量，丰富特性 | 体积大，编译复杂 |
| **lv_font_conv + FA6合并** | 需要语义精确图标（气象/医疗/工业等） | 中文字体 + FA6 OTF + 生成 .c | 0 | 图标+中文单字体单label | 需下载FA6字库，加图标要重跑 |

**两级决策**：
1. 图标 ≤10 个且 LV_SYMBOL 能覆盖 → 用 LV_SYMBOL（零成本）
2. 需要语义精确匹配（温度计、水滴、仪表等） → `lv_font_conv` 合并 FA6 + 中文字体

---

## 合并字体：中文 + Font Awesome 6 图标

**优势**：FA6 Free Solid 有 2000+ 图标，覆盖气象/医疗/工业/家居等所有场景。合并后图标和中文在**同一个字体同一个 label** 中渲染，不再需要图标+中文分离两个 label。

### Step 0: 下载 FA6 字库

```bash
# 下载 FA6 Free 桌面版 (含 OTF)
curl -L -o fa6.zip "https://use.fontawesome.com/releases/v6.7.2/fontawesome-free-6.7.2-desktop.zip"
unzip fa6.zip "fontawesome-free-*-desktop/otfs/*" -d fa6_otfs/

# 复制 Solid 字库到项目（Solid 包含 2000+ 图标）
cp fa6_otfs/fontawesome-free-*-desktop/otfs/"Font Awesome 6 Free-Solid-900.otf" \
   src/ui/fonts/FA6-Free-Solid-900.otf
```

> FA6 字库仅用于**字体生成阶段**，不需要部署到板子。生成的 `.c` 文件已包含所需 glyph。

### Step 1: 查找图标码位

从 FA6 metadata 或 [Font Awesome 图标库](https://fontawesome.com/icons) 按英文关键词搜索：

| 关键词 | 搜索 | 找到的图标 | 码位 |
|--------|------|-----------|------|
| temperature | `temperature` | temperature-three-quarters | `U+F2C8` |
| humidity/water | `droplet` / `water` | droplet | `U+F043` |
| pressure/gauge | `gauge` | gauge | `U+F624` |
| wind/air | `wind` | wind | `U+F72E` |
| refresh/sync | `rotate` / `sync` | arrows-rotate | `U+F021` |

**查码位命令**（从 FA6 zip 中提取 metadata）：
```bash
unzip -p fa6.zip "*/metadata/icons.json" | python3 -c "
import json, sys
data = json.load(sys.stdin)
for name, info in data.items():
    if 'wind' in name or 'wind' in json.dumps(info).lower():
        print(f'{name}: U+{info[\"unicode\"].upper()} — {info[\"label\"]}')
"
```

### Step 1.5: Unicode 范围速查（防止缺字）

`-r` 参数可堆叠任意多个范围，覆盖不同语言区块：

```bash
lv_font_conv --font YourFont.otf \
  -r 0x20-0x7F \       # ASCII（数字、字母、标点）
  -r 0x0080-0x00FF \   # Latin-1（° ± × ÷ 等）
  -r 0x2000-0x27BF \   # 标点 + 箭头 + 数学 + 装饰（✓ ✗ • → ≤ ∑）
  -r 0x3000-0x303F \   # CJK 标点（。、「」）
  -r 0xFF00-0xFFEF \   # 全角字符
  -r 0x4E00-0x9FFF \   # CJK 统一汉字（全部常用汉字 ~21k）
  ...
```

| 常见缺字 | 码位 | 修复 |
|----------|------|------|
| 摄氏度 ° | U+00B0 | `-r 0x0080-0x00FF` |
| 对勾 ✓ 叉号 ✗ | U+2713/2717 | `-r 0x2000-0x27BF` |
| 中文句号 。 | U+3002 | `-r 0x3000-0x303F` |
| AI 对话的任意汉字 | U+4E00 起 | `-r 0x4E00-0x9FFF` |

> ⚠️ **全量汉字 ~21k 字符，16px≈18MB，24px≈1MB。需在 lv_conf.h 启用 `LV_FONT_FMT_TXT_LARGE 1`**

### Step 2: 生成合并字体

`lv_font_conv` 支持多 `--font` 源，每个源指定各自的 `--symbols` / `-r` 范围：

```bash
lv_font_conv \
  --font SourceHanSansSC-Normal.otf -r 0x20-0x7F -r 0x0080-0x00FF \
     -r 0x2000-0x27BF -r 0x3000-0x303F -r 0xFF00-0xFFEF -r 0x4E00-0x9FFF \
  --font FA6-Free-Solid-900.otf --symbols "$FA6_ICONS" \
  --size 24 --bpp 4 --format lvgl --no-compress \
  --lv-include lvgl/lvgl.h \
  --lv-font-name "lv_font_weather_24" \
  -o src/ui/fonts/weather_24.c
```

**关键规则**：每个 `--font` 后必须有 `-r` 或 `--symbols`。`-r` 是全局 ASCII 范围，`--symbols` 是特定字符。

### Step 3: 代码中使用

合并后图标和中文在同一字体，**一个 label 搞定**：

```c
/* C 源码中直接写 FA6 图标码位（UTF-8 编码） */
#define FA6_TEMP   "\xEF\x8B\x88"  /* U+F2C8 */
#define FA6_DROPLET "\xEF\x80\x83" /* U+F043 */

/* 图标+中文混排，单 label */
lv_label_set_text(label, FA6_TEMP " 温度");

/* 不再需要两个 label 分开用 montserrat + 中文字体 */
```

也可以直接用 `\u` 转义（需要编译器支持 C11+）：
```c
#define FA6_TEMP   ""
lv_label_set_text(label, " 温度");
```

---

## lv_font_conv 完整流程

### Step A: 收集字符

在页面的 ASCII 布局设计中确定所有中文文本后，用 Python 扫描收集：

```bash
python3 -c "
import os, codecs
cjk = set()
for f in os.listdir('src/ui/pages'):
    if f.endswith(('.c','.h')):
        for ch in open(f'src/ui/pages/{f}', encoding='utf-8').read():
            if '一' <= ch <= '鿿':
                cjk.add(ch)

# 固定补充：空格（中英混排必备），如需其他符号（如 °）也在此添加
fixed = ' '

with open('src/ui/fonts/chars.txt', 'w', encoding='utf-8') as out:
    out.write(fixed + ''.join(sorted(cjk)))
"
echo "Total chars: $(wc -m < src/ui/fonts/chars.txt)"
```

### Step B: 按字号生成字库

| 用途 | 推荐字号 |
|------|---------|
| 页面大标题 | 24px |
| 正文/标签/按钮 | 16px |
| 小字/辅助信息 | 14px |

```bash
SYMBOLS=$(cat src/ui/fonts/chars.txt)

lv_font_conv \
  --font lvgl/scripts/built_in_font/SourceHanSansSC-Normal.otf \
  --size 16 --bpp 4 --format lvgl \
  -r 0x20-0x7F --symbols "$SYMBOLS" \
  --no-compress \
  --lv-include "lvgl/lvgl.h" \
  --lv-font-name "lv_font_app_16" \
  -o src/ui/fonts/app_16.c
```

**必须加 `--no-compress`**，否则 `LV_USE_FONT_COMPRESSED` 未开启时字形读不出。

**`-r 0x20-0x7F` 提供空格、数字、大小写字母、常用标点**，保证中英混排时 "亮度 80%" 这类文本正常显示。`-r` 和 `--symbols` **必须同时使用**。

### WSL 1 注意事项

`lv_font_conv` 是 Node.js 工具，WSL 1 不支持原生安装 Node.js，需通过 Windows 侧的 Node.js 运行，用 **PowerShell** 作为中转：

```bash
# 1. 先把文件拷到 Windows 原生路径
PROJDIR=~/26粤嵌园区实训/02-lv_port_linux/fhb_lv_port_linux
mkdir -p /mnt/d/temp_lv_font_conv
cp "$PROJDIR/lvgl/scripts/built_in_font/SourceHanSansSC-Normal.otf" /mnt/d/temp_lv_font_conv/
cp "$PROJDIR/src/ui/fonts/chars.txt" /mnt/d/temp_lv_font_conv/

# 2. 用 PowerShell 调用 Windows 的 node.exe 执行 lv_font_conv
SYMBOLS=$(cat /mnt/d/temp_lv_font_conv/chars.txt)
powershell.exe -Command "cd D:\\temp_lv_font_conv; D:\\nodejs\\node.exe D:\\nodejs\\node_modules\\lv_font_conv\\lv_font_conv.js --font SourceHanSansSC-Normal.otf --size 16 --bpp 4 --format lvgl -r 0x20-0x7F --symbols $SYMBOLS --no-compress --no-prefilter --lv-font-name lv_font_app_16 -o app_16.c"

# 3. 拷回项目
cp /mnt/d/temp_lv_font_conv/app_16.c "$PROJDIR/src/ui/fonts/"
```

> 如果已在 Linux 环境或 WSL 2 中，直接用 `lv_font_conv` 命令即可，无需上述 workaround。

### Step C: 字体头文件

```c
// src/ui/fonts/app_fonts.h
#ifndef APP_FONTS_H
#define APP_FONTS_H
#include "lvgl/lvgl.h"
extern const lv_font_t lv_font_app_16;
extern const lv_font_t lv_font_app_24;
#endif
```

### Step D: 代码中使用

```c
#include "src/ui/fonts/app_fonts.h"
lv_obj_set_style_text_font(title, &lv_font_app_24, 0);
lv_obj_set_style_text_font(label, &lv_font_app_16, 0);
```

---

## CMakeLists.txt 集成

```cmake
file(GLOB LV_PAGE_SRC src/ui/pages/*.c)
file(GLOB LV_FONT_SRC src/ui/fonts/*.c)
add_executable(lvglsim src/main.c ${LV_PAGE_SRC} ${LV_FONT_SRC})
```

---

## 常见字体错误

| 现象 | 根因 | 修复 |
|------|------|------|
| 中文乱码/缺字 | 字库里没这个字 | 重跑 Python 扫描 → chars.txt → 重新生成 |
| 字体不显示+控件乱跳 | 缺少 `--no-compress` | lv_font_conv 命令必须加 `--no-compress` |
| cjk 字体格式 0 无 glyph | 用 `-r 0x20-0x7F` 代替 `--symbols` | `-r` 和 `--symbols` **必须同时使用** |
| charmap 字体总大小 0 | 只改了 chars.txt 没重新生成 .c | 必须先 `lv_font_conv` 再 cmake |
| 掉电符（°C）显示方框 | ° 不在 chars.txt 里 | 用中文「度」代替，或把 ° 加入 chars.txt |
