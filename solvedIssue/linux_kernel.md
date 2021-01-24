> 此文档属于解决类文档，直接解决遇到的问题。

### 1. devm_前缀函数，无法自动释放申请的资源

比如，i2c总线下的xxx设备，在probe()函数中申请irq资源

```c
int xxx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    // 正确
    struct device *dev = &client->dev; // 注意

    devm_request_threaded_irq(dev, irq, NULL, xxx_irq_handler,
        IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "xxxIRQ", &xxxDev);
    
    /***********************************************************/
    
    // 错误
    struct device dev = client->dev; // 注意

    devm_request_threaded_irq(&dev, irq, NULL, xxx_irq_handler,
        IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "xxxIRQ", &xxxDev);
}
```

原因：devm_前缀函数会在dev中存储申请的资源，在申请资源出错或卸载驱动时，自动释放。

### 2. can't open /dev/ttyAMA0: No such file or directory

```bash
$ make menuconfig
Device Drivers  --->
	Generic Driver Options  --->
	[*]   Automount devtmpfs at /dev, after the kernel mounted the rootfs
```

### 3. gcc: error: elf_x86_64: No such file or directory

```bash
diff --git a/arch/x86/vdso/Makefile b/arch/x86/vdso/Makefile
index 6b4ffedb93c9..dd78ef687c5e 100644
--- a/arch/x86/vdso/Makefile
+++ b/arch/x86/vdso/Makefile

-VDSO_LDFLAGS_vdso.lds = -m elf_x86_64 -Wl,-soname=linux-vdso.so.1 \
+VDSO_LDFLAGS_vdso.lds = -m64 -Wl,-soname=linux-vdso.so.1 \

-VDSO_LDFLAGS_vdso32.lds = -m elf_i386 -Wl,-soname=linux-gate.so.1
+VDSO_LDFLAGS_vdso32.lds = -m32 -Wl,-soname=linux-gate.so.1
```

### 4. error: curses.h: No such file or directory

```bash
# (可选) 功能：查找某一些库或文件在哪一个deb包
$ apt install apt-file
$ apt-file update
$ apt-file search curses.h # ubuntu 14.04
libncurses-dev: /usr/include/curses.h
$ apt install libncurses-dev
or
$ apt-file search curses.h # ubuntu 16.04
libncurses5-dev: /usr/include/curses.h
$ apt install libncurses5-dev
```
