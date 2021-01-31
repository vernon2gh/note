### 步骤

查找`slub.c`的第一个提交记录

```bash
$ git log --reverse --oneline mm/slub.c
81819f0fc828 SLUB core
```

从`slub.c`的第一个提交记录`81819f0fc828`的上一个提交创建新分支`slub`

```bash
$ git branch slub 81819f0fc828~1
```

获得`slub`到目前为止的所有patch

```bash
$ i=0; for line in `git log --reverse --pretty=format:"%h" mm/slub.c`; do i=$[$i+1]; git format-patch -1 --start-number $i $line -o patches/; done
```

将第一个patch打在`slub`分支，进行研究

```bash
$ git checkout slub

$ git apply --check patches/0001-SLUB-core.patch
$ git am patches/0001-SLUB-core.patch
```

将第二个patch打在`slub`分支，进行研究

```bash
$ git apply --check patches/0002-SLUB-change-default-alignments.patch
$ git am patches/0002-SLUB-change-default-alignments.patch
```

依此类推，第xx个patch打在`slub`分支，进行研究...

### linux kernel网址

https://lore.kernel.org/
