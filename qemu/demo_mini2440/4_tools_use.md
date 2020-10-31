### ubuntu 下播放 yuv 格式的文件

* 安装ffmpeg

```shell
$ sudo apt install ffmpeg
```

* 执行如下命令

```shell
$ ffplay -f rawvideo -video_size 640x480 test_input_640x480.yuv
```

### ftrace调试linux kernel

环境：linux2.6.32

* linux kernel支持 ftrace

```
Kernel hacking  --->
	[*] Tracers  --->
		[*] Kernel Function Tracer
	-*- Debug Filesystem
```

* 通过 debugfs 访问 ftrace

将如下内容添加到 /etc/fstab 文件：

```
debugfs /sys/kernel/debug debugfs defaults 0 0 
```

或者在运行时挂载：

```shell
$ mount -t debugfs nodev /sys/kernel/debug 
```

* ftrace 操作概述

```shell
$ cd /sys/kernel/debug/tracing/
$ echo 0 > tracing_enabled
$ cat available_tracers 
function nop
$ echo function > current_tracer
$ echo 40000 > buffer_size_kb

# 建议：以下三个命令需要合并为一条命令执行
$ echo 1 > tracing_enabled
$ #运行可执行程序
$ echo 0 > tracing_enabled

$ cat trace
```

* 参考网址

[使用 ftrace 调试 Linux 内核，第 1 部分](https://blog.csdn.net/Tommy_wxie/article/details/7340701)

[使用 ftrace 调试 Linux 内核，第 2 部分](https://blog.csdn.net/Tommy_wxie/article/details/7340710)

[使用 ftrace 调试 Linux 内核，第 3 部分](https://blog.csdn.net/Tommy_wxie/article/details/7340712)

