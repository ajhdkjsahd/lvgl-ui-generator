# Flex vs Grid 布局选择指南

LVGL v9 两种布局引擎，根据场景二选一。

## 决策树

```
内容是单行/单列？
├── 是 → Flex (create_inline_row)
└── 否 → 二维矩阵？
          ├── 是 → Grid + LV_GRID_FR(1)
          └── 否 → Flex COLUMN 嵌套 Flex ROW
```

## Flex（弹性盒子）

**适合：一维排列**

```c
// 横向行：图标+文字、开关行、滑块行、底部按钮栏
lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

// 纵向列：卡片内图标→标题→描述 从上到下
lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
```

| Flex 场景 | FLOW | ALIGN |
|-----------|------|-------|
| 图标 + 文字 同行 | `ROW` | `START, CENTER, CENTER` |
| 卡片内纵向堆叠 | `COLUMN` | `CENTER, CENTER, CENTER` |
| 开关行（标签+开关） | `ROW` | `SPACE_BETWEEN, CENTER, CENTER` |
| 底部按钮栏 | `ROW` | `SPACE_EVENLY, CENTER, CENTER` |
| 换行流（标签云） | `ROW_WRAP` | `START, CENTER, CENTER` |

### 高级 Flex 属性

```c
lv_obj_set_flex_grow(child, 1);    // 伸展权重，兄弟间按比例分配剩余空间
```

## Grid（网格）

**适合：二维矩阵排列**

```c
lv_obj_set_size(grid, 776, 330);
static int32_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
static int32_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, col, 1,
                            LV_GRID_ALIGN_STRETCH, row, 1);
```

| Grid 场景 | 行列 | 列宽/行高 |
|-----------|------|-----------|
| 场景卡片 2×4 | `row_dsc={FR,FR}` `col_dsc={FR×4}` | 均分 |
| 仪表盘 2×2 | `row_dsc={FR,FR}` `col_dsc={FR,FR}` | 均分 |
| 设置菜单 5×1 | `row_dsc={FR×5}` `col_dsc={FR}` | 单列多行 |
| 不等宽布局 | `col_dsc={FR(2), FR(1)}` | 2:1 比例 |

### Grid 描述符值

| 值 | 含义 |
|----|------|
| `LV_GRID_FR(n)` | 按比例分配剩余空间 |
| `LV_GRID_CONTENT` | 适应内容大小 |
| `LV_GRID_TEMPLATE_LAST` | 数组结束标记 |
| 固定像素值（如 `200`） | 固定列宽/行高 |

---

## Grid 常见陷阱

### `lv_obj_set_grid_cell` 必须设在直接子对象上

Grid cell 的 target **必须是 grid 容器的直接子对象**，不能是孙子对象：

```c
// ✅ 正确：card 是 grid 的直接子对象
lv_obj_t * card = lv_obj_create(grid);
lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

// ❌ 错误：value_label 是 card 的子对象，不是 grid 的直接子对象
lv_obj_t * value_label = lv_label_create(card);
lv_obj_set_grid_cell(value_label, ...);  // 不会生效！
```

**当 card 由工厂函数创建时，确保返回 card 引用**：

```c
typedef struct {
    lv_obj_t * card;        // ← 必须保留，用于 set_grid_cell
    lv_obj_t * value_label;
} card_ctx_t;

static card_ctx_t create_card(lv_obj_t * parent, ...) {
    card_ctx_t ctx;
    ctx.card = lv_obj_create(parent);  // card 是 grid 的直接子对象
    ctx.value_label = lv_label_create(ctx.card);
    return ctx;
}
```

---

## Grid 卡片内描述太长

单行文字超过卡片宽度时：用 `\n` 拆成两行。`LV_LABEL_LONG_CLIP` 模式下 `\n` 依然生效。
