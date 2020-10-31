### 简介

在研究linux 0.11此类old linux时，因为历史原因过于久远，最新ubuntu 20.04的gcc版本过于高，很难编译通过，所以为了减少不必要的编译麻烦，需要用老ubuntu对应的老gcc版本来编译old linux源码

于是此处采用docker安装ubuntu 12.04环境。

### 下载

在ubuntu20.04运行如下命令

```bash
$ docker pull ubuntu:12.04
```

### 启动

在ubuntu 20.04启动docker运行ubuntu 12.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 12.04 /mnt目录

```bash
$ docker run -itd --name oldlinux -v ~/workplaces:/mnt ubuntu:12.04 bash

$ docker exec -it oldlinux bash
```

### 安装必需工具

```bash
$ apt-get update
$ apt-get install bin86
```

接下来就是安装gcc，此处安装gcc 3.4版本 

从 http://old-releases.ubuntu.com/ubuntu/pool/universe/g/gcc-3.4/ 下载如下*.deb文件，并且放在amd64目录中

```bash
$ cd amd64
$ tree
.
├── cpp-3.4_3.4.6-6ubuntu5_amd64.deb
├── g++-3.4_3.4.6-6ubuntu5_amd64.deb
├── gcc-3.4_3.4.6-6ubuntu5_amd64.deb
├── gcc-3.4-base_3.4.6-6ubuntu5_amd64.deb
└── libstdc++6-dev_3.4.6-6ubuntu5_amd64.deb
$ dpkg -i *.deb
```

安装完成之后，目前系统里有 gcc 4.6 和gcc 3.4 ，默认是 gcc 4.6

修改系统默认gcc为gcc 3.4

```bash
$ update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 40
$ update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-3.4 30
$ update-alternatives --config gcc # 选择gcc3.4版本
```

测试gcc 3.4编译main.c是否正常使用，出现如下问题，并附上修复过程

```bash
$ gcc main.c
/usr/bin/ld: cannot find crt1.o: No such file or directory
/usr/bin/ld: cannot find crti.o: No such file or directory
/usr/bin/ld: cannot find -lgcc_s
collect2: ld returned 1 exit status

# 解决 cannot find crt1.o
$ find / -name crt1.o
/usr/lib/x86_64-linux-gnu/crt1.o
$ vim .bashrc
export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LIBRARY_PATH
$ source .bashrc

# 解决 cannot find -lgcc_s
$ find / -name libgcc_s.so
/usr/lib/gcc/x86_64-linux-gnu/3.4.6/libgcc_s.so
$ ll /usr/lib/gcc/x86_64-linux-gnu/3.4.6/libgcc_s.so
lrwxrwxrwx 1 root root      18 May  8  2008 libgcc_s.so -> /lib/libgcc_s.so.1
$ find / -name libgcc_s.so.1
/lib/x86_64-linux-gnu/libgcc_s.so.1
$ ln -sf /lib/x86_64-linux-gnu/libgcc_s.so.1 /usr/lib/gcc/x86_64-linux-gnu/3.4.6/libgcc_s.so
$ ll /usr/lib/gcc/x86_64-linux-gnu/3.4.6/libgcc_s.so
lrwxrwxrwx 1 root root      35 Jun  9 00:59 libgcc_s.so -> /lib/x86_64-linux-gnu/libgcc_s.so.1
```

### 保存docker image

```bash
$ docker commit -m "mesg" <CONTAINER ID> oldlinux:latest
```

其中，`mesg`是此次提交的信息，如`Init: oldlinux`等等

`<CONTAINER ID>`是容器ID，需要通过`docker ps -a`可知

以后，执行如下命令即可使用

```bash
$ docker run -itd --name oldlinux -v ~/workplaces:/mnt oldlinux:latest bash

$ docker exec -it oldlinux bash
```

