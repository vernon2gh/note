[TOC]

### 零星知识点

* make命令的使用

执行`make`命令时，它会去当前目录下查找名为`Makefile`的文件，并根据它的指示去执行操作，生成第一个目标

我们可以使用`-f`选项指定文件，不再使用名为`Makefile`的文件，比如：

```makefile
make -f Makefile.build
```

我们可以使用`-C`选项指定目录，切换到其他目录里去，比如：

```makefile
make -C a/ -f Makefile.build
```

我们可以指定目标，不再默认生成第一个目标：

```makefile
make -C a/ -f Makefile.build other_target
```

* 即时变量与延时变量

变量的定义语法形式如下：

```makefile
A  =  xxx   # 延时变量
B  ?= xxx   # 延时变量，只有第一次定义时赋值才成功；如果曾定义过，此赋值无效
C  := xxx   # 立即变量
D  += yyy   # 如果D在前面是延时变量，那么现在它还是延时变量；
			# 如果D在前面是立即变量，那么现在它还是立即变量
```

在GNU make中对变量的赋值有两种方式：延时变量、立即变量

延时变量，它的值在使用时才展开、才确定

立即变量，它的值在定义时展开并且确定

比如：

```shell
$ cat Makefile 
A = $@
test : 
	@echo $A
$ make
test
```

上述Makefile中，变量A的值在执行时才确定，它等于test，是延时变量。

```shell
$ cat Makefile 
A := $@
test : 
	@echo $A
$ make

```

如果使用“A := $@”，这是立即变量，这时$@为空，所以A的值就是空。

* 变量的导出 ( export )

在编译程序时，我们会不断地使用`make -C dir`切换到其他目录，执行其他目录里的`Makefile`。如果想让某个变量的值在所有目录中都可见，要把它export出来。

比如`CC = $(CROSS_COMPILE)gcc`，这个CC变量表示编译器，在整个过程中都是一样的。定义它之后，要使用`export CC`把它导出来。

* Makefile 中使用shell命令

```makefile
TOPDIR := $(shell pwd)
```

这是个立即变量，TOPDIR等于shell命令pwd的结果

* 在 Makefile 中怎么放置第一个目标

执行make命令时，如果不指定目标，那么它默认是去生成第1个目标。

所以“第1个目标”，位置很重要。有时候不太方便把第1个目标完整地放在文件前面，这时可以在文件的前面直接放置目标，在后面再完善它的依赖与命令。比如：

```makefile
First_target:  # 这句话放在前面
．．．．        # 其他代码，比如include其他文件得到后面的xxx变量
First_target : $(xxx) $(yyy) # 在文件的后面再来完善
	command
```

* 假想目标

```makefile
clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)
```

如果当前目录下恰好有名为`clean`的文件，那么执行`make clean`时它就不会执行那些删除命令。

这时我们需要把`clean`这个目标，设置为“假想目标”，这样可以确保执行`make clean`时那些删除命令肯定可以得到执行。

使用下面的语句把`clean`设置为假想目标

```makefile
.PHONY : clean
```

* 常用函数

1. $(foreach var,list,text)

简单地说，就是 for each var in list, change it to text。

对list中的每一个元素，取出来赋给var，然后把var改为text所描述的形式。

例子：

```makefile
objs := a.o b.o
dep_files := $(foreach f, $(objs), .$(f).d)  # 最终 dep_files := .a.o.d .b.o.d
```

2. $(wildcard pattern)

pattern所列出的文件是否存在，把存在的文件都列出来

例子：

```makefile
src_files := $(wildcard *.c)  # 最终 src_files中列出了当前目录下的所有.c文件
```

3. $(filter pattern...,text)

把text中符合pattern格式的内容，filter(过滤)出来、留下来。

例子：

```makefile
obj-y := a.o b.o c/ d/
DIR :=  $(filter %/, $(obj-y)) # 结果为：c/ d/
```

4. $(filter-out pattern...,text)

把text中符合pattern格式的内容，filter-out(过滤)出来、扔掉。

例子：

```makefile
obj-y := a.o b.o c/ d/
DIR :=  $(filter-out %/, $(obj-y)) # 结果为：a.o  b.o
```

5. $(patsubst pattern,replacement,text)

寻找text中符合格式pattern的字，用replacement替换它们。pattern和replacement中可以使用通配符。

比如：

```makefile
subdir-y := c/ d/
subdir-y := $(patsubst %/, %, $(subdir-y)) # 结果为：c  d
```

### 通用 Makefile 的设计思想

* 在Makefile 文件中确定要编译的文件、目录

```makefile
obj-y += main.o
obj-y += a/
```

`Makefile`文件总是被`Makefile.build`包含的

* 在 Makefile.build 中设置编译规则，有3条编译规则

1. 怎么编译子目录？进入子目录编译？

```makefile
$(subdir-y):
	make -C $@ -f $(TOPDIR)/Makefile.build
```

2. 怎么编译当前目录中的文件

```makefile
%.o : %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$@) -Wp,-MD,$(dep_file) -c -o $@ $<
```

3. 当前目录下的 .o 和子目录的built-in.o 要打包起来

```makefile
built-in.o : $(cur_objs) $(subdir_objs)
	$(LD) -r -o $@ $^
```

* 顶层 Makefile 中把顶层目录的 built-in.o 链接成 APP

```makefile
$(TARGET) : built-in.o
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o
```

### 情景演绎

![1](pic\1.png)

