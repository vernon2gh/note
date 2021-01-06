## 1. make images by dockerfile

```bash
$ docker build -f <dockerfile> -t <image>:[tag] .
```

## 2. make images by commit

从容器中创建一个新镜像，并push to docker hub

```bash
$ docker commit -m "mesg" <CONTAINER ID> <image>:newtag
$ docker rmi <image>:latest
$ docker tag <image>:newtag <image>:latest
$ docker rmi <image>:newtag
$ docker push <image>:latest
```

## 3. 例子

在研究linux 0.11, linux2.6.34等版本时，因为历史原因过于久远，最新gcc版本过于高，很难编译通过，所以为了减少不必要的编译麻烦，需要用老gcc版本来编译linux源码，并且安装qemu进行仿真调试linux kernel源码。

为了将环境集成在一起，此处采用docker为x86_64/arm64安装编译与调试环境。

下载dockerfile文件对应的仓库，如下命令：

```bash
$ git clone https://github.com/vernon2gh/dockerfile.git
$ cd dockerfile
$ ls
dockerfile_linux2.x  dockerfile_linux3.x  dockerfile_linux4.x  dockerfile_linux5.x  dockerfile_oldlinux
```

制作编译与调试linux0.11的docker镜像，如下命令：

```bash
$ docker build -f dockerfile_oldlinux -t oldlinux:latest .
```

制作编译与调试linux 2.6.34/3.16/4.4/5.4的docker镜像，如下命令：

```bash
$ docker build -f <dockerfile_2/3/4/5.x> -t linux:latest .
```

**附加说明**

- old linux docker

  安装 gcc 3.4版本

- linux 2.x docker

  x86_64 安装 x86_64-linux-gnu-gcc 4.8.4/4.4 和 qemu-system-x86_64 2.0.0 版本

- linux 3.x docker

  x86_64 安装 x86_64-linux-gnu-gcc 4.8.4 和 qemu-system-x86_64 2.0.0 版本

  arm64 安装 aarch64-linux-gnu-gcc 4.9.4 和 qemu-system-aarch64 2.4.1 版本

- linux 4.x docker

  x86_64 安装 x86_64-linux-gnu-gcc 5.4.0 和 qemu-system-x86_64 2.5.0 版本

  arm64 安装 aarch64-linux-gnu-gcc 5.4.1 和 qemu-system-aarch64 2.5.0 版本

- linux 5.x docker

  x86_64 安装 x86_64-linux-gnu-gcc 7.5.0 和 qemu-system-x86_64 2.11.1 版本

  arm64 安装 aarch64-linux-gnu-gcc 7.5.0 和 qemu-system-aarch64 2.11.1 版本