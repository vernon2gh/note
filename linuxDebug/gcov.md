## 简介

gcov 全称 GNU Coverage，由 GNU 出品的代码覆盖率检查工具。

## 使用

新创建源码文件 foo.c，如下：

```c
#include <stdio.h>

void func(int flags)
{
        if (flags)
                printf("hello\n");
        else
                printf("no\n");
}

int main(int argc, char *argv[])
{
        func(1);
        func(1);

        return 0;
}
```

编译源码文件，生成 gcov 标记文件 x.gcno、可执行文件 foo

```bash
$ gcc -coverage foo.c -o foo
```

跑可执行文件 foo，生成 gcov 数据文件 x.gcda

```bash
$ ./foo
```

通过 `gcov` 工具生成 x.c.gcov 文件

```bash
$ gcov foo.c
```

打开 foo.c.gcov，可以看到每一行的覆盖信息，如下：

其中 `-` 代表无效代码行，`数字` 代表此行执行次数，`#####` 代表此行没有执行过。

```bash
$ cat foo.c.gcov
        -:    0:Source:foo.c
        -:    0:Graph:foo.gcno
        -:    0:Data:foo.gcda
        -:    0:Runs:1
        -:    1:#include <stdio.h>
        -:    2:
        2:    3:void func(int flags)
        -:    4:{
        2:    5:        if (flags)
        2:    6:                printf("hello\n");
        -:    7:        else
    #####:    8:                printf("no\n");
        2:    9:}
        -:   10:
        1:   11:int main(int argc, char *argv[])
        -:   12:{
        1:   13:        func(1);
        1:   14:        func(1);
        -:   15:
        1:   16:        return 0;
        -:   17:}
```

也可以通过 `lcov` 工具进行可视化，转换成 `index.html` 在浏览器显示，如下：

```bash
$ lcov -c -d . -o foo.info
$ genhtml foo.info -o html
```
