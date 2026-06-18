# 常见错误速查

每条都是实战验证的血泪教训。现象 → 根因 → 正确做法。

| 现象 | 根因 | 正确做法 |
|------|------|----------|
| `lv_btn_create` (v8) | API 过时 | `lv_button_create` |
| `lv_scr_act()` (v8) | API 过时 | `lv_screen_active()` |
| 中文乱码/缺字 | 字库里没这个字 | 重跑 Python 扫描 → chars.txt → lv_font_conv 重新生成 |
| 字体不显示+控件乱跳 | 缺少 `--no-compress` | lv_font_conv 命令必须加 `--no-compress` |
| 字体/页面散落根目录 | 目录不规范 | 统一放进 `src/ui/{fonts,pages}/` |
| 样式不生效 | 未 init style | 先 `lv_style_init(&style)` 再设置属性 |
| **图标显示方框** 🔥 | LV_SYMBOL 和中文字体在同一个 label | 图标+中文必须拆成两个 label，图标用 `lv_font_montserrat_*`，中文用自定义字体 |
| **Dropdown 箭头显示方框** 🔥 | `lv_obj_set_style_text_font(dd, &chinese_font, 0)` 覆盖了指示器字体 | 必须额外设置 `LV_PART_INDICATOR`：`lv_obj_set_style_text_font(dd, &lv_font_montserrat_16, LV_PART_INDICATOR)` |
| **掉电符（°C）显示方框** | ° 是 U+00B0，不在 chars.txt 里 | 直接用中文「度」代替 °C → `"26度"` |
| **cjk 字体格式 0 无 glyph** | 用 `-r 0x20-0x7F` 代替 `--symbols` 提供 ASCII | `-r 0x20-0x7F` 和 `--symbols "$SYMBOLS"` **必须同时使用** |
| **页面不该滚却滚了** | 内容总高 > 可用高度 | 精确计算布局高度预算；或合理场景下仅外层滚动 |
| **charmap 字体总大小 0** | 只改了 chars.txt 没重新生成字体 .c 文件 | cmake 只负责**编译**，不负责**生成**字体 — 必须先 `lv_font_conv` 再 cmake |
| **Grid 卡片内描述太长** | 单行文字超过卡片宽度导致截断 | 用 `\n` 拆成两行，`LV_LABEL_LONG_CLIP` 模式下 `\n` 依然生效 |
| **windows npx/unc 路径报错** | WSL 1 无法直接调用 Windows 的 node.exe | 用 PowerShell 中转（见 font-pipeline.md WSL 1 说明） |
| **点击无响应** 🔥 | 透明容器拦截了点击（见 memory: LVGL 按钮点击失灵根因分析） | 检查父容器是否设置了 `LV_OBJ_FLAG_CLICKABLE`；或设置 `LV_OBJ_FLAG_EVENT_BUBBLE` |
| **Flex 布局位置为 0** 🔥 | flex 子控件在 build 阶段坐标未就绪（见 memory: LVGL Flex 布局坐标未就绪） | 不要在 `create` 函数中立即读取坐标，在 `LV_EVENT_SCREEN_LOADED` 中读取 |
| **深色主题不生效** 🔥 | `LV_THEME_DEFAULT_DARK` 是 lv_conf 宏（默认 0=浅色），不等于 `true` | 直接传 `true`（深色）或 `false`（浅色） |
| **主题切换第二次崩溃** 🔥 | `lv_theme_delete()` 释放了控件仍在引用的样式 | 直接调用 `lv_theme_default_init` 切换，**不要**先 delete |
| **FA6 蓝牙等图标不显示** 🔥 | 对应的 FA6 图标是 Brands 风格，不在 Free Solid 字体内 | 查 `icons.json` 确认 styles，或用 Solid 中语义相近的图标代替 |
| **单个图标缺失（方框）** | 图标码位没加入 `--symbols` 列表 | `lv_font_conv` 只导出 --symbols 列出的码位，遗漏的需补入后重生成 |
| **Grid 子控件位置异常** | `lv_obj_set_grid_cell` 设在了孙控件上，而非 grid 的直接子对象 | grid cell 的 target 必须是 grid 容器的**直接子对象** |
| **透明行拦截按钮点击** 🔥 | `create_inline_row` 未清除 `LV_OBJ_FLAG_CLICKABLE` | 工厂函数加 `lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE)` |
| **°C 摄氏度显示方框** 🔥 | `°` (U+00B0) 不在 ASCII `-r 0x20-0x7F` 范围内 | 加 `-r 0x0080-0x00FF`（Latin-1 Supplement） |
| **AI 对话 ✓ 等符号缺失** | 装饰符号不在 CJK 范围内 | 加 `-r 0x2000-0x27BF`（General Punctuation + Dingbats） |
| **全量汉字编译报错** | 字体超过 256KB 需开启 Large Font 支持 | `lv_conf.h` 设 `LV_FONT_FMT_TXT_LARGE 1` |
