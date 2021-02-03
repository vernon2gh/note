现象：当需要在docker shell中执行`mount`命令时，出现如下：

```bash
$ mount [-t fstype] device dir
mount: permission denied
```

解决：在docker启动容器时，加上`--privileged`，如下：

```bash
$ docker run -itd --name slub -v ~/workplaces:/mnt --privileged vernon2dh/slub bash
```
