## codex 简介

Codex 是一个命令行 AI 编程助手，旨在帮助开发者编写、调试和优化代码。

* 通过 NPM 安装 Codex

```bash
$ nvm install node
$ npm install -g @openai/codex
```

* 原生安装

```bash
curl -fsSL https://chatgpt.com/codex/install.sh | sh
```

Codex 提供以下认证选项：

* ChatGPT App    : 使用 ChatGPT 账户登录，订阅 Plus/Pro 计划，获得包含 Codex 和 Web 界面的统一订阅。

## 使用 GPT 模型进行认证

由于 Codex 是 Openai 为了 GPT 大模型而编写的命令行客户端，
默认就是支持 GPT 大模型的，只需要简单登录 ChatGPT pro 账号，即可使用。

```bash
$ codex ## 登录 ChatGPT pro 账号，默认第一次使用 Codex 时自动执行
›
```

## 使用 deekseek 模型进行认证

为了满足 Codex 使用 deekseek 大模型，通过以下简单的配置，即可将 DeepSeek
API 接入到 GPT API 生态中。

```bash
xxx
```

## 使用 GLM 模型进行认证

为了满足 Codex 使用 GLM 大模型，通过以下简单的配置，即可将 GLM
API 接入到 GPT API 生态中。

```bash
xxx
```

## 如何使用？

```bash
$ codex   ## 进入指定目录，执行命令
> xxx     ## 进行提问即可
```

## 参考

* https://developers.openai.com/codex/cli
