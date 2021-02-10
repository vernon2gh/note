* linux下如何调试Makefile?

```bash
$ make --debug=all ##输出所有的调试信息
or
$ make -n          ## 只打印命令
```

* makefile预定义的变量

查看所有预定义变量的当前值。

```bash
$ make -p
```

各种预定义变量的解释

```makefile
${MAKE}   ## make命令
```

* @符号的使用

通常makefile会将其执行的命令行在执行前输出到屏幕上。如果将`@`添加到命令行前，这个命令将不被make回显出来。

```bash
$ vim Makefile
@echo test1
echo  test2
$ make
test1
echo test2
test2
```

* ifneq语句

判断语句，用于比较两个参数，如果两个参数不等，则该语句通过

```bash
$ vim Makefile
# 如果a和b不相等，则do something
ifneq ($(a), $(b))
    # do something
endif
```

* filter语句

过滤语句，过滤不符合指定模式的内容，仅保留符合指定模式的内容。

filter命令在过滤时，区分大小写

```bash
$ vim Makefile
sources := a b c d f g
# 指定的模式为 a b c ，多个模式间，用空格区分
$(filter a b c , $(sources))
# 上式返回值为
# a b c
```
