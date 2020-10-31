### undefined reference to symbol 'timer_settime@@GLIBC_2.3.3

This problem occurs because rt library doesn't link. 

Edit Makefile.target in your qemu directory, find `LIBS+=-lz`, add `LIBS+=-lrt` beneath this line.

Or say, added following patch locally

```c
--- a/tools/qemu-xen/Makefile.target    2013-04-05 23:39:54.000000000 +0000
+++ b/tools/qemu-xen/Makefile.target    2013-04-25 13:54:59.360000000 +0000
@@ -206,6 +206,7 @@
obj-$(CONFIG_NO_KVM) += kvm-stub.o
obj-y += memory.o
LIBS+=-lz
+LIBS+=-lrt

QEMU_CFLAGS += $(VNC_TLS_CFLAGS)
QEMU_CFLAGS += $(VNC_SASL_CFLAGS)
```

### ./stdio.h:477:1: error: 'gets' undeclared here (not in a function)

vim buildroot-2012.05/output/build/host-m4-1.4.16/lib/stdio.h +477
_GL_WARN_ON_USE (gets, “gets is a security hole - use fgets instead”); 这一行，然后把这个替换成：

```c
 #if defined(__GLIBC__) && !defined(__UCLIBC__) && !__GLIBC_PREREQ(2, 16)
_GL_WARN_ON_USE (gets, "gets is a security hole - use fgets instead");
 #endif
```

### Unescaped left brace in regex is illegal here in regex; marked by <-- HERE in m/\${ <-- HERE ([^ \t=:+{}]+)}/ at /home/vernon/workplace/qemu-mini2440/buildroot-2012.05/output/host/usr/bin/automake line 4113.

是host/bin/automake的4113行报错
因为新版的perl不在支持左大括号的使用，
进入这个文件删掉大括号，问题解决。

### gcc/doc/cppopts.texi:811:@itemx must follow @item

解决办法是将shell texinfo降级到4.13

通过下载编译源码文件可以安装比较老一点的版本：

```shell
wget http://ftp.gnu.org/gnu/texinfo/texinfo-4.13a.tar.gz
tar -zxvf texinfo-4.13a.tar.gz
cd texinfo-4.13
./configure
make
sudo make install
```


### Can't use 'defined(@array)' (Maybe you should just omit the defined()?) at kernel/timeconst.pl line 373.

```
$ vim buildroot-2012.05/output/build/linux-3.3.7/kernel/timeconst.pl +373
if (!(@val)) {	#修改此行
   @val = compute_values($hz);
}
```

### serve_image.c:32:18: error: storage size of ‘hints’ isn’t known

```
$ vim buildroot-2012.05/output/build/host-mtd-1.4.9/serve_image.c
#define __USE_XOPEN2K	#添加
```



