[TOC]

## 简述

在Linux中使用make命令来编译程序，特别是大程序；而make命令所执行的动作依赖于Makefile文件。最简单的Makefile文件如下：

```makefile
hello : hello.c
	gcc -o hello hello.c
clean :
	rm -f  hello
```

然后直接执行make命令即可编译程序，执行“make clean”即可清除编译出来的结果。

make命令根据文件更新的时间戳来决定哪些文件需要重新编译，这使得可以避免编译已经编译过的、没有变化的程序，可以大大提高编译效率。

## 为什么需要Makefile
高效地编译程序，修改源文件或头文件，只需要重新编译牵涉到的文件，就可以重新生成APP

## Makefile其实挺简单

一个简单的Makefile文件包含一系列的“规则”，其样式如下：

```makefile
目标(target) : 依赖(prerequiries)…
<tab>命令(command)
```

如果“依赖文件”比“目标文件”更加新，那么执行“命令”来重新生成“目标文件”。

命令被执行的2个条件：依赖文件比目标文件新，或是 目标文件还没生成。

## Makefile的2个函数

* $(foreach var,list,text)

  简单地说，就是 for each var in list, change it to text。
  对list中的每一个元素，取出来赋给var，然后把var改为text所描述的形式。

  ```makefile
  objs := a.o b.o
  dep_files := $(foreach f, $(objs), .$(f).d)  # 最终 dep_files := .a.o.d .b.o.d
  ```

* $(wildcard pattern)
  pattern所列出的文件是否存在，把存在的文件都列出来。

  ```makefile
  src_files := $(wildcard *.c)  # 最终src_files中列出了当前目录下的所有.c文件
  ```

## 效率高，精炼，支持自动检测头文件

```makefile
objs := main.o sub.o

test : $(objs)
	gcc -o test $^

# 需要判断是否存在依赖文件
# .main.o.d .sub.o.d
dep_files := $(foreach f, $(objs), .$(f).d)
dep_files := $(wildcard $(dep_files))

# 把依赖文件包含进来
ifneq ($(dep_files),)
  include $(dep_files)
endif

%.o : %.c
	gcc -Wp,-MD,.$@.d  -c -o $@  $<

clean:
	rm *.o test -f

distclean:
	rm  $(dep_files) *.o test -f
```

## 参考文档

gunmake.htm