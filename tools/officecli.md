## 简介

OfficeCLI 让任何 AI 智能体完全掌控 Word、Excel 和 PowerPoint，无需安装
Office，零依赖，全平台运行。

## 安装

```bash
$ curl -fsSL https://raw.githubusercontent.com/iOfficeAI/OfficeCLI/main/install.sh | bash
or
$ npm install -g @officecli/officecli
```

## 为智能体安装 skill

```bash
$ officecli install
```

自动将 officecli 技能文件安装到检测到的所有 AI 编程助手 — Claude Code、Codex、
Pi等。

## 如何使用？

```bash
$ pi
> /skill:officecli xxx     ## 进行提问即可
```

智能体可以立即创建、读取和编辑 Office 文档，无需额外配置。

## 参考

- https://github.com/iOfficeAI/OfficeCLI
