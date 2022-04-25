### 简介

在研究linux 0.11此类old linux时，因为历史原因过于久远，最新ubuntu 20.04的gcc版本过于高，很难编译通过，所以为了减少不必要的编译麻烦，需要用老ubuntu对应的老gcc版本来编译old linux源码，于是此处采用docker安装ubuntu 12.04环境。

因为当初linus是基于80386 CPU编写的old linux，所以我们需要一个仿真平台摸拟80386，是此处采用bochs来摸拟80386。

### 环境

> 此实验在docker ubuntu 12.04中编译old linux源码，在ubuntu 20.04利用bochs运行old linux

* docker
* bochs

### 安装docker

在ubuntu 20.04中安装[docker](https://www.docker.com)，请看[官网说明](https://docs.docker.com/engine/install/ubuntu/)：

```bash
$ docker -v
Docker version 19.03.10, build 9424aeaee9
```

下载ubuntu 12.04的docker镜像

```bash
$ docker pull vernon2dh/oldlinux
```

### 编译old linux

在ubuntu 20.04启动docker运行ubuntu 12.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 12.04 /mnt目录

```bash
$ docker run -itd --name oldlinux -v ~/workplaces:/mnt vernon2dh/oldlinux bash

$ docker exec -it oldlinux bash
```

在docker ubuntu 12.04运行如下命令，编译old linux

```bash
$ cd /mnt
$ cat bootsect.s   # 屏幕打印 Hello OS world
entry _start       # 标记程序入口为_start
_start:
    mov ah,#0x03   # 读入光标位置
    xor bh,bh
    int 0x10
    mov cx,#18     # 字符串长度
    mov bx,#0x0007
    mov bp,#msg1   # 指定要显示的字符串
    mov ax,#0x07c0
    mov es,ax
    mov ax,#0x1301
    int 0x10
inf_loop:           # 相当于 while(1);
    jmp inf_loop
msg1:
    .byte   13,10   # 回车换行
    .ascii  "Hello OS world"
    .byte   13,10
.org 510
boot_flag:
    .word   0xAA55

$ as86 -0 -a -o bootsect.o bootsect.s
$ ld86 -0 -s -o bootsect bootsect.o
$ dd bs=1 if=bootsect of=Image skip=32
```

### 运行old linux

在ubuntu 20.04中安装[bochs](http://bochs.sourceforge.net)，如下命令：

```bash
$ sudo apt install bochs bochs-x
$ bochs -v
========================================================================
                       Bochs x86 Emulator 2.6.11
              Built from SVN snapshot on January 5, 2020
                Timestamp: Sun Jan  5 08:36:00 CET 2020
========================================================================
```

在bochs运行old linux，如下命令：

```bash
$ cd ~/workplaces
$ cat bochsrc.bxrc
megs: 32                                       # 32MB内存
romimage: file=$BXSHARE/BIOS-bochs-latest 
vgaromimage: file=$BXSHARE/VGABIOS-lgpl-latest

floppya: 1_44="Image", status=inserted         # 软盘A，Image存放位置，默认插入状态
#floppyb: 1_44="disk.img", status=inserted     # 软盘B
#ata0-master: type=disk, path="rootimage-hd", mode=flat, cylinders=487, heads=16, spt=63  # 硬盘0，rootfs存放位置

boot: floppy      # 默认从软盘A引导系统
#boot: disk

log: /dev/null
#log: bochsout.txt # 默认日志存放位置
#panic: action=ask
#error: action=report
#info: action=report
#debug: action=ignore

$ bochs -f bochsrc.bxrc
```

### 分析编译过程 (可选)

```bash
$ as86 -0 -a -o bootsect.o bootsect.s
$ ld86 -0 -s -o bootsect bootsect.o
```

-0 表示生成 8086 的 16 位目标程序

-a 表示生成与 GNU as 和 ld 部分兼容的代码

-s 告诉链接器 ld86 去除最后生成的可执行文件中的符号信息

```bash
$ ll bootsect
-rwxr-xr-x 1 root   root     544 5月  31 20:43 bootsect*
```

注意 bootsect 是 544 字节，而引导程序必须要正好占用一个磁盘扇区，即 512 个字节。

多了 32  个字节是 ld86 产生 Minix 可执行文件格式，所以bootsect除了文本段、数据段等以外，还包括一个 Minix 可执行文件头部，如下：

```c
struct exec {
    unsigned char a_magic[2];  //执行文件魔数
    unsigned char a_flags;
    unsigned char a_cpu;       //CPU标识号
    unsigned char a_hdrlen;    //头部长度，32字节或48字节
    unsigned char a_unused;
    unsigned short a_version;
    long a_text; long a_data; long a_bss; //代码段长度、数据段长度、堆长度
    long a_entry;    //执行入口地址
    long a_total;    //分配的内存总量
    long a_syms;     //符号表大小
};
```

执行如下命令，去掉可执行文件头部，即前32 个字节（`tools/build.c` 的功能之一）

```bash
$ dd bs=1 if=bootsect of=Image skip=32
```

生成的 Image 就是去掉可执行文件头部的 bootsect

