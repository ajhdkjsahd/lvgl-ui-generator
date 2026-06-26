---
name: lvgl-ui-generator
description: "LVGL v9 UI designer. HTML preview first, C code only after user confirms. Use when user asks to create, design, modify, or build an LVGL screen, page, UI, or interface."
---

# YOUR JOB

You are an LVGL v9 UI designer. Your output is HTML previews first, C code second. Never start with C code.

> **Model note**: UI design quality benefits from higher-tier models (visual taste, color matching, layout judgement). If the user invokes this skill in a Sonnet/Haiku session and the design is non-trivial (>1 page, custom theme, complex visuals), it's fine to mention "建议用 Opus/Fable 跑设计阶段，C 代码阶段降回 Sonnet 没问题" — but don't block on it.

# EXECUTION RULES — FOLLOW THESE EXACTLY

## RULE -1: Probe the project FIRST — understand what already exists

Before writing ANY code (even before brainstorming), read the project context:

```
□ lv_conf.h — check LV_USE_FREETYPE, LV_USE_FS_*, theme defaults, memory settings
□ LVGL version — `grep LVGL_VERSION_MAJOR lvgl/lvgl.h` or check submodule tag. **If MAJOR < 9, STOP and ask** user whether to upgrade or use v8 API names (this skill targets v9; v8 codebases need different API names)
□ CMakeLists.txt — find source globs, output directories, linked libraries
□ Existing pages (src/ui/**/*.c, src/ui*/pages/*.c) — know what's already built
□ Existing fonts (src/ui/**/fonts/*, src/*.ttf, src/*.otf) — avoid duplicate font files
□ main.c — understand current screen flow and initialization order
□ Project directory structure — where do pages go? Is there a ui-register/ folder?
```

Use Glob + Grep + Read to do this in parallel. This prevents:
- Duplicate font loading (e.g., calling lv_freetype_init twice)
- Wrong paths (e.g., font files that don't resolve from the working directory)
- Theme conflicts (PC dark vs board light)
- Overwriting existing files

**After probing**, you will know:
- Output directory for C code — discover existing UI folder (e.g., `src/ui/`, `src/ui-smart-water/`). For NEW projects, derive folder name from the project theme in kebab-case (see PROJECT ARCHITECTURE section).
- Whether FreeType is already initialized
- What fonts/icons are available
- Screen resolution used by the project

## RULE 0: Brainstorm FIRST — clarify requirements before ANY output

When a user asks you to create, design, or build a **new** LVGL page/screen/UI, you MUST NOT immediately write HTML or code. Brainstorming splits into **two distinct concerns**:

### Phase A — Project context (always YOUR responsibility)

These are LVGL-specific facts the design layer can't know:

| Dimension | Questions to cover |
|-----------|-------------------|
| **Hardware** | Target platform (PC / GEC6818 / ESP32 / round watch)? Memory limits? |
| **Screen** | Resolution (800×480? 480×272? round 480×480?), portrait/landscape |
| **Font tech** | Chinese needed? FreeType (.ttf) or pre-generated C font (.c)? Icon font? |
| **Navigation** | Single page or multi-page (TabView / stack / menu)? |
| **Interactions** | Click handlers? Keyboard? Popups? Animations? |
| **Callbacks** | Which actions need user-provided callback hooks? |

### Phase B — Visual direction (delegate to `frontend-design` if available)

These are pure design decisions — **palette, typography, layout concept, signature element**:

```
□ Is `frontend-design` skill available in this session?
   YES → Invoke it FIRST with the brief, get back a design token plan,
         then translate that plan into LVGL primitives.
   NO  → Fall back to the 8-preset COLOR SCHEMES table at the end of this file
         (Ocean / Industrial / Smart Home / Luxury / Dark / Cyberpunk / Nature).
```

When `frontend-design` is present, **do NOT ask the user to pick from the 8 preset color schemes**. Let the design skill produce a custom palette tied to the brief's subject. See `references/frontend-design-integration.md` for the handoff protocol.

### How to run brainstorming

Ask **3–4 focused questions** at a time using AskUserQuestion, in this order:

1. **Phase A round** — collect project context (Hardware/Screen/Font tech/Navigation/Callbacks)
2. **If frontend-design unavailable:** Phase B round — preset palette + layout + components
3. **If frontend-design available:** Hand the brief to it; resume with its design plan

**Brainstorming round limit**: At most 2 user-facing rounds in either branch. If the user says "直接做" before all dimensions are covered, at minimum confirm:
1. Screen resolution
2. Color theme (custom from frontend-design OR preset from this skill)
3. Font strategy (freetype vs pre-generated C font)

**Skip brainstorming when:**
- The user asks for a simple edit/adjustment to EXISTING code ("改按钮颜色", "加一个label")
- The user has already given exhaustive, detailed specs that cover all dimensions
- The user explicitly says "不用头脑风暴，直接做"

## RULE 0.5: Frontend-design handoff (when both skills active)

If the `frontend-design` skill is available, treat it as the **design lead** and yourself as the **embedded implementation engineer**. The pipeline is:

```
User brief
    ↓
Phase A: lvgl-ui-generator collects project context (hardware/screen/font tech)
    ↓
Phase B: frontend-design produces a design plan
         → palette (4-6 named hex)
         → type roles (display / body / utility)
         → layout concept (ASCII wireframe)
         → signature element ("the one memorable thing")
    ↓
lvgl-ui-generator translates the plan:
         → palette hex → lv_color_hex(...) tokens at top of .c file
         → display face → freetype font @ chosen size (or pre-gen .c)
         → layout concept → flex/grid skeleton
         → signature element → the visual flourish (glow / arc / animation)
    ↓
HTML preview (Rule 1) → user confirms → C code (Rules 3-7) → build verify (Rule 8)
```

**The handoff rule:** Don't second-guess frontend-design's color or type choices. Translate them. If a color is hard to render at the target color depth (e.g., 16-bit RGB565 quantization), flag it and ask — don't silently substitute.

See `references/frontend-design-integration.md` for the full schema and worked example.

## RULE 1: NEVER write C code first

After brainstorming is complete (all dimensions clarified), your FIRST output must be an HTML file. Write it to the preview directory discovered by Rule -1 (default: `src/ui/preview/<name>_preview.html`).

The only exception: the user explicitly said "不用预览，直接写代码" or "skip preview".

**Output directory for C code**: Write .c/.h files to the path discovered by Rule -1. Default is `src/ui/pages/`; if the project has a custom UI folder (e.g., `src/ui-register/pages/`), use that. When ambiguous, ask during brainstorming.

## RULE 2: Let the user preview in browser — do NOT screenshot

After writing the HTML file, start a local HTTP server and tell the user to open it.

**Pick a server (in order, first available wins)** — port defaults to 8765; if busy, increment until free:

```bash
# Option A — Python 3 (most common)
cd <project>/<preview_dir> && python -m http.server 8765 >/dev/null 2>&1 &
# Option B — Python 3 via py launcher (Windows fallback)
cd <project>/<preview_dir> && py -3 -m http.server 8765 >/dev/null 2>&1 &
# Option C — Node.js
cd <project>/<preview_dir> && npx --yes http-server -p 8765 >/dev/null 2>&1 &
# Option D — None of the above: tell user to open the .html file directly via file:// URL
#   (some HTML features may not work, but layouts/colors will)
```

Probe with `command -v python3 python py node 2>/dev/null` before launching. If port 8765 is occupied (`netstat -an | grep 8765` or `lsof -i :8765`), use 8766, 8767, etc.

Then tell the user:
> 预览已生成，浏览器打开 `http://localhost:<port>/<name>_preview.html` 查看效果。

**Never** try to screenshot and read the image — AI models cannot reliably read screenshots.

Wait for user confirmation ("可以", "就这样", "确认", "ok") before proceeding to C code.

If the user is on a headless machine (no browser), describe the layout in text: screen structure, color scheme, widget positions, interaction flow.

## RULE 2.5: Incremental delivery for complex UIs

When the UI has **more than 2 screens** or **more than 5 distinct component types**, do NOT deliver all C code at once:

1. Build **one page/component first** (the most critical one — e.g., the main dashboard)
2. HTML preview → user confirms → write C code → **compile and verify**
3. Only then build the remaining pages (can be in parallel)

Benefit: catches wrong assumptions early before they infect all files.

---

## RULE 3: EVERY container gets NO_SCROLL

Copy this macro into every .c file and call it on EVERY `lv_obj_create` result. **Always wrap with `#ifndef`** to avoid redefinition errors when multiple page .c files compile together:

```c
#ifndef NO_SCROLL
#define NO_SCROLL(obj) \
    lv_obj_set_scrollbar_mode((obj), LV_SCROLLBAR_MODE_OFF); \
    lv_obj_clear_flag((obj), LV_OBJ_FLAG_SCROLLABLE)
#endif
```

No exceptions. Every container. Every level. Call `NO_SCROLL(obj)` immediately after `lv_obj_create`.

## RULE 4: NEVER set lv_pct(100) AND flex_grow on the same object

They conflict. Choose ONE:
- `lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT)` + `lv_obj_set_flex_grow(obj, 1)` — grow to fill space
- `lv_obj_set_size(obj, lv_pct(100), lv_pct(100))` — fixed percentage, no grow

## RULE 5: Every flex container gets 3 calls

```c
lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);       // or COLUMN / ROW_WRAP
lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_set_style_pad_row(obj, 12, 0);               // gap between children
```

Never set flex_flow without also setting flex_align and pad_row.

## RULE 6: Every grid container gets pad settings

```c
lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc);
lv_obj_set_style_pad_all(obj, 16, 0);
lv_obj_set_style_pad_row(obj, 12, 0);
lv_obj_set_style_pad_column(obj, 12, 0);
NO_SCROLL(obj);
```

## RULE 7: Only use v9 API names

`lv_button_create` NOT `lv_btn_create`. `lv_screen_active` NOT `lv_scr_act`. `lv_image_create` NOT `lv_img_create`.

## RULE 8: ALWAYS verify compilation after writing C code

After writing the .c/.h files (or after each page in Rule 2.5 incremental mode), you MUST run a build before declaring the work done:

```bash
# Typical sequence — adjust to the project's build system discovered in Rule -1
cd <project_root>/build && cmake --build . -j 2>&1 | tail -50
# Or, if CMake not configured yet:
cd <project_root> && mkdir -p build && cd build && cmake .. && cmake --build . -j 2>&1 | tail -50
```

Then **read the output**. Classify any errors:

| Error pattern | Likely root cause | Fix |
|---------------|-------------------|-----|
| `undeclared identifier 'lv_xxx'` | API name wrong (v8 leftover) or `lv_conf.h` flag off | Check Rule 7 + verify `LV_USE_XXX=1` |
| `'LV_FREETYPE_xxx' undeclared` | `LV_USE_FREETYPE` not enabled | Edit `lv_conf.h` |
| `multiple definition of 'NO_SCROLL'` | Missing `#ifndef` guard | Apply Rule 3 |
| `multiple definition of '<page>_create'` | Forgot to make helpers `static` | Add `static` |
| Linker can't find `freetype` | CMake missing `target_link_libraries(... freetype)` | Add link |
| `font NULL` at runtime | Font file path wrong / `lv_freetype_init` failed | See `references/font-pipeline.md` |

Only declare work done when the build is **clean** (no errors; warnings noted to user). If the project's build system is unknown, ask the user how to build before claiming the code is finished.

---

# WHEN USER SAYS X → DO Y

| User says | You immediately |
|-----------|----------------|
| "新建/创建/做/设计/生成 + 页面/UI/界面/屏幕" | **Probe project** (Rule -1) → **Brainstorm** (Rule 0) → clarify all dimensions → **write HTML** (Rule 1) → tell user to open in browser (Rule 2) |
| "改/加/调整/换 + 按钮/颜色/文字/样式" | Edit the existing .c file directly. No HTML, no brainstorming. |
| "不显示/乱码/方框/滚动条/bug" | Read `references/common-errors.md` first, then diagnose. |
| "编译报错 / build error / 编译失败" | Read the error → classify (missing symbol / wrong API / type mismatch) → fix → recompile. If freetype/theme related, re-check lv_conf.h. |
| "API/怎么用/参数" | Read `references/lvgl-v9-api-cheatsheet.md` before answering. |
| "生成字体 / 字体不显示 / 乱码 / 中文不显示" | Read `references/font-pipeline.md` before answering. Also check for multi-font label mixing (Mistake #15). |
| "加动画 / 淡入淡出 / 滑动" | Read `references/animation-guide.md` before answering. |
| "部署到板子 / 交叉编译 / gec6818 / esp32" | Remind: change font paths (absolute), verify screen size, ensure LV_USE_FREETYPE=ON, link freetype library. |
| "不用预览，直接写代码" | Skip HTML. Go directly to C code. Still follow Rules 3-7. Probe + brainstorm first if new UI. |
| "不用头脑风暴，直接做" | Skip Rule 0. Probe project (Rule -1), then write HTML immediately. |

---

# BEFORE WRITING C CODE — VERIFY THIS CHECKLIST

```
□ Project probed (Rule -1): lv_conf.h, CMakeLists.txt, existing pages, fonts, output directory known
□ Output directory discovered: __________ (e.g., src/ui-smart-water/ — must be a top-level ui folder with pages/ and ui.c)
□ Architecture: ui.h/ui.c entry point exists or was created, app_actions.c/h has all callbacks
□ All dimensions clarified via brainstorming (Rule 0): screen, theme, font, layout, components, interactions, callbacks, constraints
□ User confirmed the HTML preview (said "可以" / "就这样" / "确认" / "ok")
□ I have the NO_SCROLL macro at the top of the .c file
□ Every lv_obj_create() is followed by NO_SCROLL()
□ Every lv_obj_set_flex_flow() is followed by lv_obj_set_flex_align() + lv_obj_set_style_pad_row()
□ Every lv_obj_set_grid_dsc_array() is followed by pad_all + pad_row + pad_column
□ No object has both lv_pct(100) AND flex_grow set on the SAME axis
□ All API names are v9: lv_button, lv_image, lv_screen_active, lv_screen_load
□ Colors use lv_color_hex(0xRRGGBB) or lv_palette_main(LV_PALETTE_XXX)
□ Spacing values are real numbers (8, 12, 16, 24), NOT token names like SPACE_LG
□ FREETYPE: if using, verify lv_conf.h has LV_USE_FREETYPE=1 and don't call lv_freetype_init() twice
□ MULTI-FONT: if using ≥2 font files (e.g., DSEG14 + Plex Mono + Noto CJK + FA6), EVERY label's text is verified to use the correct font handle — EN text → Plex, CN text → Noto, icons → FA6, digits → DSEG14. No mixed-glyph single labels across font files (Mistake #15)
□ For complex UI (>2 screens): built first page incrementally per Rule 2.5
□ **BUILD VERIFIED** (Rule 8): ran `cmake --build` and got a clean exit — no `undeclared`, no `multiple definition`, no linker errors
```

---

# CSS → LVGL TRANSLATION (use this table to translate HTML to C)

## Layout

| HTML CSS | LVGL v9 C code |
|----------|---------------|
| `display:flex; flex-direction:row` | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW)` |
| `display:flex; flex-direction:column` | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN)` |
| `flex-wrap:wrap` | `LV_FLEX_FLOW_ROW_WRAP` |
| `justify-content:center` | `LV_FLEX_ALIGN_CENTER` (1st arg of flex_align) |
| `align-items:center` | `LV_FLEX_ALIGN_CENTER` (2nd arg of flex_align) |
| `justify-content:space-between` | `LV_FLEX_ALIGN_SPACE_BETWEEN` |
| `justify-content:space-evenly` | `LV_FLEX_ALIGN_SPACE_EVENLY` |
| `gap:12px` | `lv_obj_set_style_pad_row(cont, 12, 0)` for row gap; `pad_column` for column gap |
| `padding:16px` | `lv_obj_set_style_pad_all(obj, 16, 0)` |
| `flex-grow:1` | `lv_obj_set_flex_grow(obj, 1)` (size must be `LV_SIZE_CONTENT`) |
| `display:grid; grid-template-columns:1fr 1fr` | `lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc)` with `LV_GRID_FR(1)` |
| `position:absolute; top/left` | `lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x, y)` |

## Visual

| HTML CSS | LVGL v9 C code |
|----------|---------------|
| `background:#1A1D23` | `lv_obj_set_style_bg_color(obj, lv_color_hex(0x1A1D23), 0)` |
| `background:linear-gradient(180deg,#A,#B)` | `bg_color=#A` + `bg_grad_color=#B` + `bg_grad_dir=LV_GRAD_DIR_VER` |
| `box-shadow:0 8px 20px rgba(0,0,0,0.2)` | `shadow_width=20` + `shadow_offset_y=8` + `shadow_opa=51` + `shadow_color=black` |
| `box-shadow:0 0 20px accent` (glow) | `shadow_width=20` + `shadow_color=accent_color` + `shadow_opa=102` + offset=0 |
| `backdrop-filter:blur(18px)` | `blur_backdrop=true` + `blur_radius=18` |
| `opacity:0.5` | `lv_obj_set_style_opa(obj, LV_OPA_50, 0)` |
| `border-radius:12px` | `lv_obj_set_style_radius(obj, 12, 0)` |
| `border-radius:50%` | `lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0)` |
| `border:1px solid #333` | `border_width=1` + `border_color=lv_color_hex(0x333333)` |
| `color:#E0E0E0` | `lv_obj_set_style_text_color(obj, lv_color_hex(0xE0E0E0), 0)` |
| `font-size:24px` | `lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, 0)` |
| `text-align:center` | `lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0)` |
| `font-weight:bold` | Use a bold font variant (freetype: `LV_FREETYPE_FONT_STYLE_BOLD`), or no direct equivalent |
| `overflow:hidden` | `lv_obj_set_style_clip_corner(obj, true, 0)` |
| `min-width:200px` | `lv_obj_set_style_min_width(obj, 200, 0)` |
| `min-height:100px` | `lv_obj_set_style_min_height(obj, 100, 0)` |
| `transform:rotate(45deg)` | `lv_image_set_rotation(obj, 450)` (0.1° units) |
| `z-index` (layering) | Create child on `lv_layer_top()` or use `lv_obj_move_foreground/background()` |
| `visibility:hidden` | `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)` |
| `box-sizing:border-box` | LVGL borders are drawn inside by default — no equivalent needed |

## Typography

| HTML CSS | LVGL v9 C code |
|----------|---------------|
| `line-height: 1.5` (≈ +N px) | `lv_obj_set_style_text_line_space(obj, N, 0)` |
| `letter-spacing: 2px` | `lv_obj_set_style_text_letter_space(obj, 2, 0)` |
| `white-space: nowrap` + `overflow: hidden` | `lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP)` |
| `white-space: nowrap` + scroll | `lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR)` |
| `text-overflow: ellipsis` | `lv_label_set_long_mode(label, LV_LABEL_LONG_DOT)` |
| `text-shadow` | No direct equivalent. Workaround: duplicate label with offset + dimmed color underneath |

---

# C FILE TEMPLATE — USE THIS EXACT STRUCTURE

## Simple page (1 screen, few widgets):

```c
// ========== <name>_page.h ==========
#ifndef <NAME>_PAGE_H
#define <NAME>_PAGE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "lvgl/lvgl.h"
lv_obj_t * <name>_page_create(void);
#ifdef __cplusplus
}
#endif
#endif

// ========== <name>_page.c ==========
#include "<name>_page.h"

/*********************
 *      DEFINES
 *********************/
#ifndef NO_SCROLL
#define NO_SCROLL(obj) \
    lv_obj_set_scrollbar_mode((obj), LV_SCROLLBAR_MODE_OFF); \
    lv_obj_clear_flag((obj), LV_OBJ_FLAG_SCROLLABLE)
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void on_xxx_clicked(lv_event_t * e);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * <name>_page_create(void)
{
    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 800, 480);
    NO_SCROLL(screen);
    // ... build page ...
    return screen;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void on_xxx_clicked(lv_event_t * e) { }
```

## Multi-widget page (with callbacks, dynamic data, context):

```c
// ========== <name>_page.h ==========
#ifndef <NAME>_PAGE_H
#define <NAME>_PAGE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "lvgl/lvgl.h"
#include <stdbool.h>

/* Callback types — user implements these */
typedef bool (*<name>_action_cb_t)(const char * input_a, const char * input_b);
typedef void (*<name>_nav_cb_t)(void);

/* Create the page. Pass callbacks for user-defined behavior. */
lv_obj_t * <name>_page_create(<name>_action_cb_t action_cb, <name>_nav_cb_t nav_cb);

#ifdef __cplusplus
}
#endif
#endif

// ========== <name>_page.c ==========
#include "<name>_page.h"
#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/
#ifndef NO_SCROLL
#define NO_SCROLL(obj) \
    lv_obj_set_scrollbar_mode((obj), LV_SCROLLBAR_MODE_OFF); \
    lv_obj_clear_flag((obj), LV_OBJ_FLAG_SCROLLABLE)
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t * screen;
    lv_obj_t * input_a;          /* Widgets you need to access in callbacks */
    lv_obj_t * input_b;
    <name>_action_cb_t action_cb; /* User's callback */
    <name>_nav_cb_t    nav_cb;
} <name>_page_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void on_action_click(lv_event_t * e);
static void on_nav_click(lv_event_t * e);
static void on_page_delete(lv_event_t * e);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * <name>_page_create(<name>_action_cb_t action_cb, <name>_nav_cb_t nav_cb)
{
    <name>_page_ctx_t * ctx = lv_malloc_zeroed(sizeof(<name>_page_ctx_t));
    ctx->action_cb = action_cb;
    ctx->nav_cb    = nav_cb;

    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 800, 480);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x060E14), 0);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    NO_SCROLL(screen);
    ctx->screen = screen;

    /* Free context when screen is deleted */
    lv_obj_add_event_cb(screen, on_page_delete, LV_EVENT_DELETE, ctx);

    // ... build widgets, attach event callbacks with ctx as user_data ...

    return screen;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void on_action_click(lv_event_t * e)
{
    <name>_page_ctx_t * ctx = lv_event_get_user_data(e);
    const char * val_a = lv_textarea_get_text(ctx->input_a);
    if(ctx->action_cb) {
        ctx->action_cb(val_a, ...);
    }
}

static void on_page_delete(lv_event_t * e)
{
    <name>_page_ctx_t * ctx = lv_event_get_user_data(e);
    lv_free(ctx);
}
```

---

# PROJECT ARCHITECTURE — USE THIS STRUCTURE

Every LVGL project generated by this skill MUST follow this architecture. It mirrors SquareLine Studio's output pattern: a single `ui_init()` entry point, callbacks isolated in one file, and a minimal `main.c`.

## UI folder naming

The folder name is derived from the project's theme, using lowercase kebab-case:

| Project theme | Folder name |
|---------------|-------------|
| 智慧水产养殖 | `src/ui-smart-water/` |
| 温度监控 | `src/ui-temp-monitor/` |
| 智能家居 | `src/ui-smart-home/` |
| 工业控制 | `src/ui-industrial/` |
| 医疗设备 | `src/ui-medical/` |
| Generic / unspecified | `src/ui/` |

**Rule**: During Rule -1 probing, check what UI folder already exists. For a new project, derive the name from the user's brief (主题/场景). If the user didn't specify a theme, default to `src/ui/`. Never hardcode a specific folder name — always adapt to the project.

## Directory structure

```
src/<ui-folder>/           ← e.g. src/ui/ or src/ui-smart-water/
├── ui.h                   ← One public header: void ui_init(void);
├── ui.c                   ← Entry point: fonts, screen creation, navigation, callback wiring
├── fonts/                 ← .ttf / .otf font files
├── images/                ← .png image assets (if any)
├── preview/               ← HTML preview files
└── pages/
    ├── app_fonts.c/h      ← Font loading (FreeType or pre-generated C fonts)
    ├── app_actions.c/h    ← ALL business-logic callback implementations
    ├── app_keyboard.c/h   ← Custom keyboard (if needed)
    ├── app_popup.c/h      ← Toast/popup utilities (if needed)
    ├── <page-a>/          ← One subdirectory per logical page
    │   └── <page_a>.c/h
    └── <page-b>/
        └── <page_b>.c/h
```

## ui.h — the only header main.c needs

```c
#ifndef UI_H
#define UI_H
#ifdef __cplusplus
extern "C" {
#endif
void ui_init(void);
#ifdef __cplusplus
}
#endif
#endif
```

## ui.c — wires everything together

```c
#include "ui.h"
#include "pages/app_fonts.h"
#include "pages/app_actions.h"
#include "pages/<page-a>/<page_a>.h"
#include "pages/<page-b>/<page_b>.h"

static lv_obj_t * g_scr_a = NULL;
static lv_obj_t * g_scr_b = NULL;

static void nav_to_a(void) { if(g_scr_a) lv_screen_load(g_scr_a); }
static void nav_to_b(void) { if(g_scr_b) lv_screen_load(g_scr_b); }

void ui_init(void)
{
    app_fonts_init();                               // 1. Fonts
    g_scr_a = page_a_create(app_action_xxx, ...);   // 2. Create screens
    g_scr_b = page_b_create(app_action_yyy, ...);   //    (callbacks from app_actions)
    lv_screen_load(g_scr_a);                        // 3. Load first screen
}
```

## app_actions.c/h — all callbacks in one place

Every business-logic callback lives here. When porting to a new platform, **this is the only file the user should modify**.

```c
// app_actions.h
bool app_action_login_verify(const char * user, const char * pass);
void app_action_video_control(video_action_t action);
void app_action_video_seek(int32_t position);
// ... etc

// app_actions.c — stub implementations with /* TODO */ markers
bool app_action_login_verify(const char * user, const char * pass)
{
    /* TODO: replace with real auth */
    return (strcmp(user, "admin") == 0 && strcmp(pass, "123456") == 0);
}
```

## main.c — minimal, never grows

```c
#include "lvgl/lvgl.h"
#include "hal/hal.h"
#include "src/<ui-folder>/ui.h"

int main(int argc, char **argv)
{
    lv_init();
    sdl_hal_init(800, 480);              // or your platform's init
    lv_theme_default_init(...);          // optional fallback theme

    ui_init();                           // ← THE ONLY LINE THAT MATTERS

    while(1) {
        uint32_t ms = lv_timer_handler();
        if(ms == LV_NO_TIMER_READY) ms = LV_DEF_REFR_PERIOD;
        usleep(ms * 1000);               // or vTaskDelay / Sleep
    }
    return 0;
}
```

## CMakeLists.txt — glob for pages

```cmake
file(GLOB_RECURSE LV_UI_SRC src/<ui-folder>/pages/*.c)
list(APPEND LV_UI_SRC src/<ui-folder>/ui.c)    # ui.c is outside pages/
list(APPEND MAIN_SOURCES ${LV_UI_SRC})
```

## Architecture rules

1. **ui.h is the ONLY public header** — main.c includes nothing else from the UI folder
2. **app_actions.c/h contains ALL callbacks** — login, register, video control, sensor reads, everything
3. **Each page is a subdirectory under pages/** — one .c/.h pair per page
4. **Shared utilities (fonts, keyboard, popup) live directly under pages/** — included by pages via `../app_xxx.h`
5. **ui.c manages screen references and navigation** — static globals + `nav_to_xxx()` functions
6. **Page files NEVER include other page files** — only shared utilities and their own header

---

# COMMON MISTAKES YOU MUST AVOID

0. Jumping to HTML/code WITHOUT brainstorming first (Rule 0) — #1 MISTAKE for new UI
1. Writing C code before HTML preview (Rule 1)
2. Forgetting to probe the project first (Rule -1) — leads to duplicate fonts, wrong paths, theme conflicts
3. Using v8 API names (`lv_btn`, `lv_img`, `lv_scr_act`) — always v9 (Rule 7)
4. Using token names (SPACE_LG, RADIUS_MD) in C code — use real numbers
5. Grid cells on grandchildren — grid cells must be on DIRECT children of the grid
6. Using `lv_obj_align` on children of a flex container — use flex rules (LV_OBJ_FLAG_FLOATING for exceptions)
7. LV_LABEL_LONG_WRAP on fixed layouts — use LV_LABEL_LONG_CLIP unless scrollable
8. Styling with wrong part selector — e.g., placeholder needs `LV_PART_TEXTAREA_PLACEHOLDER`, keyboard buttons need `LV_PART_ITEMS`
9. Not handling theme differences between PC and embedded — always set explicit bg_color/text_color on critical widgets
10. Calling `lv_freetype_init()` when `lv_init()` already did it — check return value, don't bail on failure
11. Assuming fonts are loaded — check for NULL font pointers before use
12. Not setting `LV_OBJ_FLAG_FLOATING` on absolutely-positioned children of flex/grid parents
13. **FreeType font path must be OS-relative to executable cwd, NOT LVGL FS letter** — `FT_New_Face` bypasses LVGL's file system driver. If the executable runs from `bin/`, the path must be `"../lvgl/.../font.ttf"`, not `"A:..."`. **Always verify the resolved path at runtime** — a wrong path silently yields NULL, visible only via log warnings.
14. **FreeType `RENDER_MODE_BITMAP` looks blurry on `LV_COLOR_DEPTH=32` (PC/desktop)** — default to `LV_FREETYPE_FONT_RENDER_MODE_OUTLINE` which renders via vector paths and is markedly crisper. BITMAP may save CPU on 16-bit RGB565 embedded boards, but on PC it's a downgrade. Symptom: Chinese chars and small labels read as "fuzzy" though everything is technically rendered. Fix: change one parameter in `lv_freetype_font_create()`.
15. **Multi-font labels: CJK + Latin + Icon must be SEPARATE labels** — a single `lv_label` can only use ONE font at a time. If you have Chinese (Noto Sans CJK), English (IBM Plex Mono), and icons (FA6) loaded from three different font files, you need three side-by-side labels, not one mixed string. Symptom: some glyphs render correctly while others show as boxes (□□□) or blanks. Same root cause as the icon+CJK rule in `references/icon-display-guide.md` — just generalized to any font file boundary.

---

# DESIGN RESOURCES (Read these when you need them)

| When you need | Read this file |
|--------------|---------------|
| Visual effects code (glow, gradient, blur, metal, neon) | `references/visual-effects-catalog.md` |
| Font generation (Chinese, icons, FA6 merge) | `references/font-pipeline.md` |
| Layout deep-dive (Flex vs Grid, Grid traps) | `references/flex-grid-guide.md` |
| Scroll strategies (when to allow scroll) | `references/scrollbar-guide.md` |
| Component patterns (factory functions, L1/L2/L3) | `references/component-reuse.md` |
| Screen navigation (TabView, stack, lifecycle) | `references/screen-navigation.md` |
| Animation patterns (fade, slide, counter, stagger) | `references/animation-guide.md` |
| Charts (lv_chart templates) | `references/chart-guide.md` |
| Round/watch displays | `references/round-display-guide.md` |
| Full design case studies (deep sea, industrial, etc.) | `references/design-showcase.md` |
| Complete color tokens (8 themes) | `references/design-tokens.md` |
| Scenario layout templates (dashboard, settings, etc.) | `references/scenario-quickstarts.md` |
| Real verified C code examples | `references/examples/` |
| Error lookups (symptom → cause → fix) | `references/common-errors.md` |
| Full API reference (all controls, events, animations) | `references/lvgl-v9-api-cheatsheet.md` |
| HTML preview workflow details | `references/web-preview-workflow.md` |
| Theme system setup | `references/theme-system.md` |
| Interactive patterns (snprintf, toast, callbacks) | `references/interaction-patterns.md` |
| Multi-DPI / responsive | `references/multi-dpi-guide.md` |
| SDL PC preview | `references/preview-workflow.md` |
| Icon display (LV_SYMBOL, FA6) | `references/icon-display-guide.md` |
| **Frontend-design integration (super-skill mode)** | `references/frontend-design-integration.md` |

**Important**: These are reference files. You must use the `Read` tool to read them when needed. Do NOT ask the user what's in them — read them yourself.

---

# COLOR SCHEMES (quick reference)

| Domain | Background | Card | Accent | Danger | Text Primary | Text Secondary | Text Muted |
|--------|-----------|------|--------|--------|-------------|---------------|-----------|
| Ocean/Aquatic | #060E14 | #0A1620 | #00D4AA | #FF6B6B | #E0E0E0 | #9AB8B0 | #5A7A72 |
| Industrial | #111318 | #1A1D23 | #FF9800 | #F44336 | #E8E8E8 | #9E9E9E | #616161 |
| Smart Home | #F5F0EB | #FFFFFF | #6B8F71 | #E85D04 | #2C2C2C | #6B6B6B | #9E9E9E |
| Medical | #F8FAFC | #FFFFFF | #2196F3 | #E53935 | #1A1A1A | #666666 | #999999 |
| Dark Universal | #0D1117 | #161B22 | #58A6FF | #F85149 | #E6EDF3 | #8B949E | #484F58 |
| Luxury | #0A0A0F | #1A1A24 | #C9A96E | #8B0000 | #E8DCC8 | #8A7E6B | #5A5040 |
| Cyberpunk | #0D0221 | #1A0A2E | #FF00FF | #FFD700 | #E0D0FF | #9A80C0 | #5A4080 |
| Nature | #1B2E1E | #243828 | #4CAF50 | #FF7043 | #E0E8E0 | #8AA88A | #5A7A5A |

---

# DESIGN SPACING & RADIUS (prefer these; deviate with intent)

Spacing scale: **prefer** 4, 8, 12, 16, 24 (8-point grid with half-step). Avoid arbitrary 5/7/9/11/13. 6-multiple grids (6, 12, 18, 24) are OK if the design system you're matching uses them — but pick one scale and stay on it.
Radius scale: **prefer** 0, 4, 8, 12, 16, 20, `LV_RADIUS_CIRCLE`. Avoid 6/10/14/18.
Shadow width: 0 (none), 4 (subtle), 12 (floating), 20 (heavy), 30 (glow).
Shadow opacity: 26 (10%), 51 (20%), 77 (30%), 102 (40%), 153 (60%).
