[TOC]

### 参考书

![1](pic\1.png)

这2本书的内容类似，第一本对知识点有更细致的描述，适合初学者；

第二本比较直接，一上来就是各种函数的介绍，适合当作字典，不懂时就去翻看一下。

做纯Linux应用的入，看这2本书就可以了，不需要学习我们的视频。我们的侧重于“嵌入式Linux”。

在Linux系统中，一切都是“文件”：普通文件、驱动程序、网络通信等等。所有的操作，都是通过“文件IO”来操作的。所以，很有必要掌握文件操作的常用接口。

### 文件从哪来？

![2](pic\2.png)

### 怎么访问文件？

* 通用的IO模型：open/read/write/lseek/close
* 不是通用的函数：ioctl/mmap

### 怎么知道这些函数的用法？

Linux下有3大帮助方法：help、man、info。

想查看某个命令的用法时，比如查看ls命令的用法，可以执行：

```shell
ls --help
```

help只能用于查看某个命令的用法，而man手册既可以查看命令的用法，还可以查看函数的详细介绍等等。它含有9大分类，如下：

```c#
1   Executable programs or shell commands       	 	// 命令
2   System calls (functions provided by the kernel)  	// 系统调用，比如 man 2 open
3   Library calls (functions within program libraries)  // 函数库调用
4   Special files (usually found in /dev)             	// 特殊文件, 比如 man 4 tty 
5   File formats and conventions eg /etc/passwd  		// 文件格式和约定, 比如man 5 passwd
6   Games  												// 游戏
7   Miscellaneous (including macro packages and conventions), e.g. man(7), groff(7) //杂项
8   System administration commands (usually only for root) // 系统管理命令
9   Kernel routines [Non standard]  					// 内核例程
```

比如想查看open函数的用法时，可以直接执行“man open”，发现这不是想要内容时，再执行“man  2  open”。

在man命令中可以及时按“h”查看帮助信息了解快捷键。常用的快捷键是：

```
f  往前翻一页
b  往后翻一页
/patten 往前搜
?patten 往后搜
```

就内容来说，info手册比man手册编写得要更全面，但man手册使用起来更容易些。

以书来形容info手册和man手册的话，info手册相当于一章，里面含有若干节，阅读时你需要掌握如果从这一节跳到下一节；而man手册只相当于一节，阅读起来当然更容易。

就个人而言，我很少使用info命令。

可以直接执行“info”命令后，输入“H”查看它的快捷键，在info手册中，某一节被称为“node”，常用的快捷键如下：

```
Up          Move up one line.
Down        Move down one line.
PgUp        Scroll backward one screenful.
PgDn        Scroll forward one screenful.
Home        Go to the beginning of this node.
End         Go to the end of this node.

TAB         Skip to the next hypertext link.
RET         Follow the hypertext link under the cursor.
l           Go back to the last node seen in this window.

[           Go to the previous node in the document.
]           Go to the next node in the document.
p           Go to the previous node on this level.
n           Go to the next node on this level.
u           Go up one level.
t           Go to the top node of this document.
d           Go to the main 'directory' node.
```

### 系统调用函数怎么进入内核？

![3](pic\3.png)

### 内核的sys_open、sys_read会做什么？

![4](pic\4.png)



