### 移植strace

从https://github.com/strace/strace下载strace-4.10.tar.gz，解压缩后

执行如下命令：

 ```shell
$ ./bootstrap
$ ./configure --host=arm-linux CC=arm-linux-gcc LD=arm-linux-ld
$ make
$ arm-linux-strip ./strace # 去掉一些调试信息
 ```

将strace下载到MINI2440开发板的/usr/sbin目录下，即可使用







