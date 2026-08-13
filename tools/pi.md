## 简介

Pi 是一个命令行 AI 编程助手，旨在帮助开发者编写、调试和优化代码。

* 通过 NPM 安装

```bash
$ nvm install node
$ npm install -g --ignore-scripts @earendil-works/pi-coding-agent
```

如果提示权限或找不到命令错误，不要使用 `sudo npm install -g`，因为这可能导致
权限问题和安全风险。

* 原生安装

```bash
$ curl -fsSL https://pi.dev/install.sh | sh
```

## 登陆认证

```bash
$ pi
> /login  ## 登陆账号/key
> /model  ## 选择大模型
```

## 如何使用？

```bash
$ pi      ## 进入指定目录，执行命令
> xxx     ## 进行提问即可
```

## 配置

使用 `pi install npm:<package>` 安装 extensions, skills, 主题等

```bash
$ pi install npm:@termdraw/pi                       ## 绘图
$ pi install npm:@czottmann/pi-automode             ## 权限自动模式
$ pi install npm:@quintinshaw/pi-dynamic-workflows  ## workflows
$ pi install npm:@99percentpeople/pi-ssh-remote     ## SSH
$ pi install npm:pi-mcp-adapter                     ## MCP
$ pi install npm:pi-lsp                             ## LSP
$ pi install npm:pi-zentui                          ## TUI
```

## 参考

- https://pi.dev/
- https://pi.dev/packages
