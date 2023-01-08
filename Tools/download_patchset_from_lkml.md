我们经常在 LKML 中阅读新 feature 对应的 patchset，有时候想要提前品尝一下效果，
这时候我们要如何从 LKML 中下载新 feature 对应的 patchset？

1. 安装 b4

```bash
$ sudo apt install b4
or
$ git clone git://git.kernel.org/pub/scm/utils/b4/b4.git
```

2. 从 [LKML](https://lore.kernel.org/) 中找到想要的 patchset
3. 从 permalink 找到 Message-ID
4. 通过 `b4 am <Message-ID>` 下载 patchset
5. 得到一个 `*.mbx` 后缀的文件，此文件就是 patchset
6. 通过 `git am *.mbx` 将 patchset 打上 linux 源码中
