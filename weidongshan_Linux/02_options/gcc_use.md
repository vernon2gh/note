[TOC]

## 简述

PC机上的编译工具链为gcc、ld、objcopy、objdump等，它们编译出来的程序在x86平台上运行。要编译出能在ARM平台上运行的程序，必须使用交叉编译工具xxx-gcc、xxx-ld等(不同版本的编译器的前缀不一样，比如arm-linux-gcc)，下面分别介绍。

## GCC编译过程(精简版)

一个C/C++文件要经过预处理(preprocessing)、编译(compilation)、汇编(assembly)和链接(linking)等4步才能变成可执行文件。

![](pic\图片1.png)

在日常交流中通常使用“编译”统称这4个步骤，如果不是特指这4个步骤中的某一个，本教程也依惯例使用“编译”这个统称。

```shell
cc1 main.c -o /tmp/ccXCx1YG.s
as         -o /tmp/ccZfdaDo.o /tmp/ccXCx1YG.s

cc1 sub.c -o /tmp/ccXCx1YG.s
as         -o /tmp/ccn8Cjq6.o /tmp/ccXCx1YG.s

collect2 -o test /tmp/ccZfdaDo.o /tmp/ccn8Cjq6.o ....
```

## 常用编译选项
在学习时，我们暂时只需要了解下表中的选项。

| 常用选项 | 描述                                                |
| -------- | --------------------------------------------------- |
| -E       | 预处理，开发过程中想快速确定某个宏可以使用“-E  -dM” |
| -c       | 把预处理、编译、汇编都做了，但是不链接              |
| -o       | 指定输出文件                                        |
| -I       | 指定头文件目录                                      |
| -L       | 指定链接时库文件目录                                |
| -l       | 指定链接哪一个库文件                                |

### 怎么编译多个文件

* 一起编译、链接：

  ```shell
  gcc  -o test  main.c  sub.c
  ```

* 分开编译，统一链接：

  ```shell
  gcc -c -o main.o  main.c
  gcc -c -o sub.o   sub.c
  gcc -o test main.o sub.o
  ```

### 制作、使用动态库

* 制作、编译：

  ```shell
  gcc -c -o main.o  main.c
  gcc -c -o sub.o   sub.c
  gcc -shared  -o libsub.so  sub.o  sub2.o  sub3.o(可以使用多个.o生成动态库)
  gcc -o test main.o  -lsub  -L /libsub.so/所在目录/
  ```

* 运行：
  先把libusb.so放到PC或板子上的/lib目录，然后就可以运行test程序。
  如果不想把libusb.so放到/lib，也可以放在某个目录比如/a，然后如下执行：

  ```shell
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/a  
  ./test
  ```

### 制作、使用静态库

* 制作、编译：

```shell
gcc -c -o main.o  main.c
gcc -c -o sub.o   sub.c
ar  crs  libsub.a  sub.o  sub2.o  sub3.o(可以使用多个.o生成静态库)
gcc  -o  test  main.o  libsub.a  (如果.a不在当前目录下，需要指定它的绝对或相对路径)
```

* 运行：
  不需要把静态库libsub.a放到板子上。

### 很有用的选项

```shell
gcc -E main.c   // 查看预处理结果，比如头文件是哪个
gcc -E -dM main.c  > 1.txt  // 把所有的宏展开，存在1.txt里
gcc -Wp,-MD,abc.dep -c -o main.o main.c  // 生成依赖文件abc.dep，后面Makefile会用
```

