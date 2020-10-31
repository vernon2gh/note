### 简介

在研究linux 2.6.34此类linux时，最新gcc版本很难编译通过，所以为了减少不必要的编译麻烦，需要用老gcc版本来编译linux 2.6.34源码，并且安装qemu进行仿真调试linux kernel源码

为了将环境集成在一起，此处采用docker ubuntu 14.04，为x86_64安装编译与调试环境。

x86_64 安装 x86_64-linux-gnu-gcc 4.8.4 和 qemu-system-x86_64 2.0.0 版本

### 下载

在ubuntu20.04运行如下命令

```bash
$ cd ~/workplaces
$ docker pull ubuntu:14.04
```

### 启动

在ubuntu 20.04启动docker运行ubuntu 14.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 14.04 /mnt目录

```bash
$ docker run -itd --name linux2.x -v ~/workplaces:/mnt ubuntu:14.04 bash

$ docker exec -it linux2.x bash
```

### 安装必需工具

```bash
$ apt update

# x86_64
$ apt install gcc g++ make libncurses-dev qemu rsync patch wget unzip bc
$ dpkg --add-architecture i386 # buildroot编译x86_64 rootfs需要32位库
$ apt update
$ apt install libc6:i386 libstdc++6:i386 zlib1g:i386

# 使用apt install gdb会出现Remote 'g' packet reply is too long错误，所以修改gdb源码，然后安装gdb
$ wget http://ftp.gnu.org/gnu/gdb/gdb-7.8.tar.xz
$ tar -xf gdb-7.8.tar.xz
$ cd gdb-7.8/
$ vim gdb/remote.c
//if (buf_len > 2 * rsa->sizeof_g_packet)
//    error (_("Remote 'g' packet reply is too long: %s"), rs->buf);

if (buf_len > 2 * rsa->sizeof_g_packet) {
    rsa->sizeof_g_packet = buf_len ;
    for (i = 0; i < gdbarch_num_regs (gdbarch); i++) {
        if (rsa->regs->pnum == -1)
           continue;
        if (rsa->regs->offset >= rsa->sizeof_g_packet)
           rsa->regs->in_g_packet = 0;
        else
           rsa->regs->in_g_packet = 1;
    }
}

$ ./configure
$ make
$ sudo make install
```

### 保存docker image

```bash
$ docker commit -m "mesg" <CONTAINER ID> linux-2.x:latest
```

其中，`mesg`是此次提交的信息，如`Init: linux-2.x`等等

`<CONTAINER ID>`是容器ID，需要通过`docker ps -a`可知

以后，执行如下命令即可使用

```bash
$ docker run -itd --name linux2.x -v ~/workplaces:/mnt linux-2.x:latest bash

$ docker exec -it linux2.x bash
```
