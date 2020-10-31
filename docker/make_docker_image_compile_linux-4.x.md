### 简介

在研究linux 4.4此类linux时，最新gcc版本很难编译通过，所以为了减少不必要的编译麻烦，需要用老gcc版本来编译linux 4.4源码，并且安装qemu进行仿真调试linux kernel源码

为了将环境集成在一起，此处采用docker ubuntu 16.04，为x86_64/arm64安装编译与调试环境。

x86_64 安装 x86_64-linux-gnu-gcc 5.4.0 和 qemu-system-x86_64 2.5.0 版本

arm64 安装 aarch64-linux-gnu-gcc 5.4.1 和 qemu-system-aarch64 2.5.0 版本

### 下载

在ubuntu20.04运行如下命令

```bash
$ cd ~/workplaces
$ docker pull ubuntu:16.04
```

### 启动

在ubuntu 20.04启动docker运行ubuntu 16.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 16.04 /mnt目录

```bash
$ docker run -itd --name linux4.x -v ~/workplaces:/mnt ubuntu:16.04 bash

$ docker exec -it linux4.x bash
```

### 安装必需工具

```bash
$ apt update

# x86_64
$ apt install gcc make libncurses5-dev bc g++ patch wget cpio python unzip rsync bzip2 perl qemu

# arm64
$ cd /opt
$ wget https://releases.linaro.org/components/toolchain/binaries/5.4-2017.05/aarch64-linux-gnu/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz
$ apt install xz-utils
$ tar -Jxvf gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz
$ vim .bashrc
export PATH=$PATH:/opt/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu/bin/
```

### 保存docker image

```bash
$ docker commit -m "mesg" <CONTAINER ID> linux-4.x:latest
```

其中，`mesg`是此次提交的信息，如`Init: linux-4.x`等等

`<CONTAINER ID>`是容器ID，需要通过`docker ps -a`可知

以后，执行如下命令即可使用

```bash
$ docker run -itd --name linux4.x -v ~/workplaces:/mnt linux-4.x:latest bash

$ docker exec -it linux4.x bash
```
