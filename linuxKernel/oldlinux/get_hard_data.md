### 简介

old linux bootsect.s有加载并运行setup.s的功能，然后在setup.s读取一些硬件参数，提供给kernel相关子系统使用。

此章是一个简单setup.s读取一些硬件参数。

### 环境

> 此实验在docker ubuntu 12.04中编译old linux源码，在ubuntu 20.04利用bochs运行old linux

* docker
* bochs

### 编译old linux

在ubuntu 20.04启动docker运行ubuntu 12.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 12.04 /mnt目录

```bash
$ docker run -itd --name oldlinux -v ~/workplaces:/mnt vernon2dh/oldlinux bash

$ docker exec -it oldlinux bash
```

在docker ubuntu 12.04运行如下命令，编译[old linux](https://github.com/vernon2gh/vernon2gh.github.io/tree/master/resources/HitOSLab/3_get_hard_data)

```bash
$ cd HitOSLab/3_get_hard_data
$ make BootImage
```

### 运行old linux

在ubuntu 20.04运行如下命令，运行old linux

```bash
$ cd HitOSLab/3_get_hard_data
$ bochs -f bochsrc.bxrc
```

