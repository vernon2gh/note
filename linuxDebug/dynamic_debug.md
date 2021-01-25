### 1. 编译linux kernel

使能dynamic debug功能

```bash
## based on linux 2.6.34 version
$ make x86_64_defconfig
$ make menuconfig
Kernel hacking  --->
	-*- Debug Filesystem                 ## CONFIG_DEBUG_FS
	[*] Enable dynamic printk() support  ## CONFIG_DYNAMIC_DEBUG
$ make
```

### 2. 启动linux kernel

挂载debugfs(可选)

```bash
$ mount -t debugfs none /sys/kernel/debug/

$ ls /sys/kernel/debug/dynamic_debug/control
```

### 3. 例子

例子一：[测试代码](../resources/patch/linuxDebug/0001-test-dynamic-debug.patch)

```bash
## 打上测试dynamic debug的补丁
$ git am 0001-test-dynamic-debug.patch
$ make

## 启动qemu，然后加载dynamic_debug_test.ko
$ insmod dynamic_debug_test.ko
$ lsmod
Module                  Size  Used by    Not tainted
dynamic_debug_test      1664  0

## 以module为单位输出log
$ echo "module dynamic_debug_test +p" > /sys/kernel/debug/dynamic_debug/control
$ dmesg | tail
[  115.749615] 1 4
[  115.749716] 2 4
[  116.751008] 2 5
[  116.751805] 1 5
[  117.753031] 2 6
[  117.753634] 1 6
[  118.755193] 2 7
[  118.755700] 1 7
[  119.756883] 2 8
[  119.757013] 1 8
## 关闭
$ echo "module dynamic_debug_test -p" > /sys/kernel/debug/dynamic_debug/control


## 以file为单位输出log
$ echo "file dynamic_debug_test.c line 13 +p" > /sys/kernel/debug/dynamic_debug/control
$ dmesg | tail
[  209.855417] 1 8
[  210.856516] 1 9
[  211.857505] 1 0
[  212.859586] 1 1
[  213.861215] 1 2
[  214.863533] 1 3
[  215.864938] 1 4
[  216.866647] 1 5
## 关闭
$ echo "file dynamic_debug_test.c line 13 -p" > /sys/kernel/debug/dynamic_debug/control


## 以function为单位输出log
$ echo "func work_func2 +p" > /sys/kernel/debug/dynamic_debug/control
$ dmesg | tail
[  360.023953] 2 8
[  361.024880] 2 9
[  362.025961] 2 0
[  363.027486] 2 1
[  364.028408] 2 2
[  365.029529] 2 3
## 关闭
$ echo "func work_func2 -p" > /sys/kernel/debug/dynamic_debug/control
```
