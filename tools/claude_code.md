## claude code 简介

Claude Code 是一个命令行 AI 编程助手，旨在帮助开发者编写、调试和优化代码。

* 通过 NPM 安装 Claude Code

```bash
$ npm install -g @anthropic-ai/claude-code
```

如果提示权限或找不到命令错误，不要使用 `sudo npm install -g`，因为这可能导致
权限问题和安全风险。

* 原生安装

```bash
curl -fsSL https://claude.ai/install.sh | bash
```

Claude Code 提供以下认证选项：

* Claude App    : 使用 Claude.ai 账户登录，订阅 Pro/Max 计划，获得包含 Claude Code 和 Web 界面的统一订阅。
* Claude Console: 默认选项，通过使用 Anthropic API 访问。

## 使用 claude 模型进行认证

由于 claude code 是 Anthropic 为了 claude 大模型而编写的命令行客户端，
默认就是支持 claude 大模型的，只需要简单登录 claude pro 账号，即可使用。

```bash
$ claude
> /login  ## 登录 claude pro 账号，默认第一次使用claude code时自动执行
```

## 使用 deekseek 模型进行认证

为了满足 claude code 使用 deekseek 大模型，通过以下简单的配置，即可将 DeepSeek
API 接入到 Anthropic API 生态中。

```bash
export ANTHROPIC_BASE_URL=https://api.deepseek.com/anthropic
export ANTHROPIC_AUTH_TOKEN=${DEEPSEEK_API_KEY}
export API_TIMEOUT_MS=600000
export ANTHROPIC_MODEL=deepseek-chat
export ANTHROPIC_SMALL_FAST_MODEL=deepseek-chat
export CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=1
```

注：设置API_TIMEOUT_MS是为了防止输出过长，触发 Claude Code
客户端超时，这里设置的超时时间为 10 分钟。

## 使用 GLM 模型进行认证

为了满足 claude code 使用 GLM 大模型，通过以下简单的配置，即可将 GLM
API 接入到 Anthropic API 生态中。

```bash
export ANTHROPIC_BASE_URL=https://open.bigmodel.cn/api/anthropic
export ANTHROPIC_AUTH_TOKEN=${GLM_API_KEY}
export API_TIMEOUT_MS=3000000
export CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=1
```

## 如何使用？

```bash
$ claude  ## 进入指定目录，执行命令
> /init   ## 生成 CLAUDE.md 文件
> xxx     ## 进行提问即可
```

## 参考

* https://docs.claude.com/zh-CN/docs/claude-code/quickstart
* https://api-docs.deepseek.com/zh-cn/guides/anthropic_api
* https://docs.bigmodel.cn/cn/coding-plan/tool/claude
