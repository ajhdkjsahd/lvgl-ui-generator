# LVGL UI Generator — Claude Code Skill

嵌入式 Linux LVGL v9 代码生成 skill，覆盖**需求分析 → 布局设计 → C 代码生成 → 中文字库 → 构建集成 → 部署**全流程。

## 适用平台

- LVGL v9 + fbdev/evdev 嵌入式 Linux
- ARM 交叉编译（已验证 GEC6818 / S5P6818）
- 800×480 等中小分辨率 TFT/LCD

## 文件结构

```
lvgl-ui-generator/
├── skill.md                          ← 主文档（工作流程 + 核心规范）
├── references/
│   ├── lvgl-v9-api-cheatsheet.md     ← 所有控件 API 签名速查
│   └── coding-conventions.md         ← 代码模板 + 命名约定 + v8→v9 迁移
└── README.md                         ← 本文件
```

## 能解决什么问题

| 模块 | 覆盖 |
|------|------|
| **布局** | Flex vs Grid 选择指南 + 决策树 + 高度预算速算 |
| **滚动条** | 根因分析 + 预防铁律 + 工厂函数 + 自检清单 |
| **图标** | LV_SYMBOL_* 与中文分离规则 + 31 个符号速查表 + Dropdown 箭头修复 |
| **中文** | lv_font_conv + chars.txt 按需生成 + WSL 1 PowerShell workaround |
| **交互** | snprintf 动态文本模板 + 事件回调三种 user_data 传参法 |
| **错误** | 14 条实战错误（现象 → 根因 → 修复） |

## 安装

```bash
# 克隆到 Claude Code 的 skills 目录
git clone https://github.com/ajhdkjsahd/lvgl-ui-generator.git \
  ~/.agents/skills/lvgl-ui-generator
```

在 Claude Code 中输入 `lvgl` 或描述 UI 需求即可自动触发。

## 参考

- [LVGL 官方文档](https://docs.lvgl.io/)
- LVGL 源码内置符号：`lvgl/include/lvgl/font/lv_symbol_def.h`
