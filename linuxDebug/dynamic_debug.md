### 0. 简介

linux 内核 支持 动态调试（dynamic debug）功能，顾名思义就是 在需要调试时，将调试功能开启，不用将调试信息写死在dmesg buffer中，避免过多或不必要的调试信息。

* 前提：

  只能使用`dev_dbg() / pr_debug()`打印调试信息，不可以用`printk()`

* 好处：

  在调试 linux 内核源码过程中，打印的调试信息不需要删除掉，默认不会将 调试信息 打印在 dmesg 中。在需要时，开启 动态调试 ，即可 将调试信息打印出来。

这里分两种情况：

1. 在linux 内核源码没有发布时，在源码最前面加上`#define DEBUG`，然后使用`dev_dbg() / pr_debug()` 就可以打印调试信息

   当驱动源码需要发布时，将`#define DEBUG`删除即可。

2. 在linux 内核源码发布后，出现问题，但是硬件环境在客户/测试手上，这时候不需要重新找一套相同的环境来调试，只需要让 客户/测试 帮忙输入某些命令，即可 将调试信息全部打印出来。

### 1. 使能 dynamic debug 功能，编译 Linux Kernel

```bash
CONFIG_DYNAMIC_DEBUG=y
```

### 2. （可选）挂载debugfs

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

## 以module为单位输出log
$ echo "module dynamic_debug_test +p" > /sys/kernel/debug/dynamic_debug/control
## 关闭
$ echo "module dynamic_debug_test -p" > /sys/kernel/debug/dynamic_debug/control

## 以file为单位输出log
$ echo "file dynamic_debug_test.c line 13 +p" > /sys/kernel/debug/dynamic_debug/control
## 关闭
$ echo "file dynamic_debug_test.c line 13 -p" > /sys/kernel/debug/dynamic_debug/control

## 以function为单位输出log
$ echo "func work_func2 +p" > /sys/kernel/debug/dynamic_debug/control
## 关闭
$ echo "func work_func2 -p" > /sys/kernel/debug/dynamic_debug/control
```
