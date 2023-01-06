我们经常在 LKML 中阅读新 feature 对应的 patchset，有时候想要提前品尝一下效果，
这时候我们要如何从 LKML 中下载新 feature 对应的 patchset？

1. 从 [LKML](https://lore.kernel.org/) 中找到想要的 patchset
2. 从 permalink 找到 Message-ID
3. 下载 b4 脚本 `$ git clone git://git.kernel.org/pub/scm/utils/b4/b4.git`
4. 下载 patchset `$ ./b4.sh am <Message-ID>`
5. 得到一个 `*.mbx` 后缀的文件，此文件就是 patchset
6. 将 patchset 打上 linux 源码中 `$ git am *.mbx`

