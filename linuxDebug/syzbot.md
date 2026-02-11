## 简述

syzbot 是由 Google 运营的、用于 Linux 内核的自动化模糊测试与漏洞报告系统。

### 核心工作方式

1.  持续测试：它在多台服务器上持续运行多个 syzkaller 内核模糊测试实例。
2.  自动复现与报告：当 syzkaller 触发一个内核崩溃后，syzbot 会自动：
    * 尝试复现该崩溃。
    * 对崩溃报告进行去重和分析。
    * 如果确认为新漏洞，会自动创建一个包含详细日志、代码回溯和可能补丁的公开错误
      报告，并通过邮件发送给相关的内核维护者和子系统邮件列表。

### 关键特点与价值

*   完全自动化：从测试到报告的全流程无需人工干预，极大地扩展了内核的测试覆盖面。
*   公开透明：所有发现的崩溃、报告、修复状态和统计信息都公开在 [syzkaller](https://syzkaller.appspot.com/upstream) 上。
*   跟踪修复：它会跟踪已报告问题的修复状态，在修复被合并后自动关闭相应的报告。
*   社区驱动：是内核安全社区的核心基础设施之一，帮助维护者快速发现和修复代码中的
              潜在安全与稳定性问题。

简而言之，syzbot 是一个 7x24 小时工作的“机器人测试员”，专门为 Linux 内核寻找和报告缺陷。

## 在本地复现 syzbot 报告的 bug

前提条件：

- 内核配置文件 `.config`
- 可复现 BUG 的复现程序（reproducer）
    - `C` 源码文件（`repro.c`）
    - 二进制格式的程序（`repro.syz`），用于在 `syz-execprog` 下运行

直接使用 `.config` 编译最新内核可执行镜像，然后使用 qemu 运行内核，最后使用
reproducer 复现 BUG。

### repro.c

```bash
$ gcc -o repro repro.c
$ ./repro
```

### repro.syz

```bash
## 编译 syz-execprog
$ sudo dnf install go libstdc++-static
$ git clone https://github.com/google/syzkaller.git
$ cd syzkaller
$ make
$ ls bin/linux_amd64/
syz-execprog syz-executor

## 执行 repro.syz
$./syz-execprog -enable=all -repeat=0 -procs=6 ./repro.syz
```
