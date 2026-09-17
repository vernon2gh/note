## 简介

review-prompts 是 AI 辅助代码审查 prompt 库，面向 Linux kernel / systemd /
iproute 三个项目。

## 安装

```bash
$ git clone https://github.com/masoncl/review-prompts.git

$ grep -rlZ --include='*.md' -- '{{REVIEW_DIR}}' "$HOME/review-prompts/kernel" \
    | xargs -0 -r sed -i "s|{{REVIEW_DIR}}|$HOME/review-prompts/kernel|g"
$ ln -s $HOME/review-prompts/kernel/slash-commands $HOME/.pi/agent/prompts
or
$ cd $HOME/review-prompts
$ ./setup.sh <agent> <project>
```

## 命令

* 单 commit 回归分析

```bash
/kreview                # 分析 HEAD
/kreview <sha>          # 分析指定 commit
```

发现回归时生成 `./review-inline.txt`（邮件列表格式，可直接发 LKML）；
每次都生成 `./review-metadata.json`。

* 整个 patchset 分析

```bash
/kseries base..head
```

对 range 内**每个 commit** 执行完整 kreview 协议，最后做跨 commit 分析。
最后生成 `<sha>/review-inline.txt`、`series-summary.txt`、`series-metadata.json`。

* 多 agent 版 kreview

```bash
/korcreview [commit|base..head]
```

输入、产出与 kreview 相同，区别是加载 review 编排器（agent/orc.md），
把审查协议拆分给多个子 agent 执行。

* 误报验证

```bash
/kverify    # 在已有分析结论的会话中使用
```

加载 false-positive-guide.md，对当前分析出的问题逐条验证、消除误报。
一般跟在 kreview 发现回归之后使用。

* 调试内核 Bug

```bash
/kdebug    # 然后贴入 crash 报告 / oops / warning / stack trace
           # 可选：复现程序（C/shell/syz）、syzbot URL
```

多 agent 编排：从崩溃报告生成 2~5 个理论，再派 code / commits / reproducer
子 agent 逐个调查（最多 12 次派发）。

中间状态保存在 `./debug-context/*.json`，最终报告为 `./debug-report.txt`。

* 主观风格检查

```bash
/kslop [commit]     # 默认分析 HEAD
```

检查 SLOP-* 风格问题（过度工程、AI 味的代码/prose），最多输出 3 条，
以提问语气给出，不指认作者。是 review 流程中主观检查阶段的独立运行版，
主要用于测试和校准。

* 生成 Coccinelle 语义补丁

```bash
/cocci <描述要做的重复性代码改动>
```

生成 `.cocci` 语义补丁并给出 `make coccicheck` 应用命令。适合跨多文件的
重命名、API 变更、样板代码删除等机械性改动。
