### 下载old linux

在ubuntu运行如下命令

```bash
$ cd ~/workplaces
$ git clone https://github.com/vernon2gh/oldlinux.git -b linux0.11
```

### 编译old linux

在ubuntu启动docker，并将ubuntu ~/workplaces目录挂载docker /mnt目录

```bash
$ docker pull vernon2dh/oldlinux
$ docker run -itd --name oldlinux -v ~/workplaces:/mnt vernon2dh/oldlinux bash
$ docker exec -it oldlinux bash
```

在docker运行如下命令，编译old linux

```bash
$ cd /mnt/oldlinux
$ make
```

### 运行old linux

在ubuntu运行如下命令，运行old linux

```bash
$ cd ~/workplaces/oldlinux
$ bochs -f bochsrc.bxrc
```
