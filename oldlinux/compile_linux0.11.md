### 简介

在研究linux 0.11此类old linux时，最新gcc版本很难编译通过，所以为了减少不必要的编译麻烦，需要用老gcc版本来编译old linux源码

此处采用docker ubuntu 12.04环境，安装 gcc 3.4.6

### 下载old linux

在ubuntu20.04运行如下命令

```bash
$ cd ~/workplaces
$ git clone https://github.com/vernon2gh/oldlinux.git -b linux0.11
```

### 编译old linux

在ubuntu 20.04启动docker运行ubuntu 12.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 12.04 /mnt目录

```bash
$ docker pull vernon2dh/oldlinux
$ docker run -itd --name oldlinux -v ~/workplaces:/mnt vernon2dh/oldlinux bash
$ docker exec -it oldlinux bash
```

在docker ubuntu 12.04运行如下命令，编译old linux

```bash
$ cd /mnt/oldlinux
$ make
```

### 运行old linux

在ubuntu 20.04运行如下命令，运行old linux

```bash
$ cd ~/workplaces/oldlinux
$ bochs -f bochsrc.bxrc
```
