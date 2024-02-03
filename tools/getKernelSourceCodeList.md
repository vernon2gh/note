## 问题由来
linux内核源码比较庞大，在导入到 **cscope或gtags** 时会非常耗时，而且导入后查看代码时也会遇到诸多问题，如函数的重复定义，软件崩溃等。而做嵌入式开发，往往只需要用到与具体硬件相关的代码就可以了。**那么如何从内核源码里提取到我们需要的代码呢？**


## 得到需要的所有内核源码
在配置编译内核源码前，一般要先指定好architecture和machine，例如arm vexpress开发板
architecture 是 arm
machine      是 vexpress

从以下命令行得到build.log文件
```bash
$ export ARCH=arm
$ export CROSS_COMPILE=arm-linux-gnueabi-
$ make vexpress_defconfig
	
$ make zImage -j8 >> build.log
$ make dtbs -j8 >> build.log
```

然后通过分析build.log来提取所有.c文件以及.c文件里包含的头文件，
那么就可以获得我们需要的所有内核源码了，如下命令
```bash
$ ./getKernelSourceCode arm vexpress build.log file_list.txt
```

**file_list.txt** 文件就是我们需要的所有内核源码！


## C++源码
[getKernelSourceCode.cpp](../resources/code/g++/getKernelSourceCode/getKernelSourceCode.cpp)

[Makefile](../resources/code/g++/getKernelSourceCode/Makefile)


## cscope使用file_list.txt
```bash
$ cscope -Rqkb -i file_list.txt
或者
$ mv file_list.txt cscope.files
$ cscope -Rqkb
```


## gtags使用file_list.txt
```bash
$ gtags -f file_list.txt
或
$ mv file_list.txt gtags.files
$ gtags
```


参考链接：
[精确制导 --- 把linux内核源码中需要的代码导入Source Insight](https://blog.csdn.net/whahu1989/article/details/81156304) 

