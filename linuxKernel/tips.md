## tips

* Linux Kernel 如何得到某一个 `.c` 对应的 `.o/.i/.s/.lst` 文件？

```bash
$ make some/path/file.o
$ make some/path/file.i
$ make some/path/file.s
$ make some/path/file.lst ## assembler code with the C source
```
