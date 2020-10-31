[TOC]

### 设置环境变量

``` shell
$ PATH=$PATH:/home/test/workplace/100ask_am335x/ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin
```

### 交叉编译源码

* 执行如下命令

```shell
$ arm-linux-gnueabihf-gcc hello.c -o hello
```

* 交叉编译器中头文件的默认路径

```shell
$ cd ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/
$ find . -name stdio.h
./arm-linux-gnueabihf/include/c++/6.2.1/tr1/stdio.h
./arm-linux-gnueabihf/libc/usr/include/bits/stdio.h
./arm-linux-gnueabihf/libc/usr/include/stdio.h
./lib/gcc/arm-linux-gnueabihf/6.2.1/include/ssp/stdio.h
```

* 指定头文件目录

```shell
$ arm-linux-gnueabihf-gcc hello.c -o hello -I <头文件目录>
```

* 交叉编译器中库文件的默认路径

```shell
$ cd ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/
$ find . -name lib
./arm-linux-gnueabihf/libc/lib
./arm-linux-gnueabihf/libc/usr/lib
./arm-linux-gnueabihf/lib
./lib
```

* 指定库文件目录、指定要用的库文件（如：库文件libabc.so）

```shell
$ arm-linux-gnueabihf-gcc hello.c -o hello -L <库文件目录> -l<库名字,如abc>
```

### 总结：

![](pic\图片1.png)

