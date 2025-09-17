# 需求

我们经常在 LKML 中阅读新 feature 对应的 patchset，有时候想要提前品尝一下效果，
这时候我们要如何从 LKML 中下载新 feature 对应的 patchset？

# b4

## 简介

b4 是一个专为 Linux 内核开发工作流设计的命令行工具。它的核心功能是帮助开发者通过
电子邮件高效地处理内核补丁集（patchset）。

在内核开发中，补丁不是通过 GitHub 之类的 Pull Request 提交的，而是以电子邮件的
形式发送到邮件列表进行讨论和评审。b4 工具极大地简化了接收、下载、验证和应用这些
补丁集的过程。

## 安装

```bash
$ sudo apt install b4
or
$ git clone git://git.kernel.org/pub/scm/utils/b4/b4.git
```

## 下载指定邮件

2. 从 [LKML](https://lore.kernel.org/) 中找到想要的 patchset
3. 从 permalink 找到 Message-ID
4. 通过 `b4 am <Message-ID>` 下载 patchset
5. 得到一个 `*.mbx` 后缀的文件，此文件就是 patchset
6. 通过 `git am *.mbx` 将 patchset 打上 linux 源码中

# public-inbox

## 简介

public-inbox 是 LKML 本身现在使用的工具，也可以用它来在本地克隆和同步邮件列表。

这样我们相当于得到所有内核邮件，得到内核补丁集（patchset）。

## 安装

```bash
$ sudo dnf install public-inbox
```

## 初始化 LKML

```bash
$ public-inbox-init -V2 lkml ./lkml https://lore.kernel.org/lkml linux-kernel@vger.kernel.org
```

## 同步 LKML 到本地

```bash
$ public-inbox-index ./lkml
```

## 参考

https://lore.kernel.org/lkml/_/text/mirror/
