### 编译 linux2.6.34具有debug info和顺序执行源码

第一步，开启[debug info](./debug_symbols.md)

第二步，顺序执行源码。

因为linux kernel默认编译时，会有源码优化选项`-O2`，如果想要在调试时按顺序执行源码，需要指定`-O0`， 使得编译器不做源码优化，如下：

```bash
## 修改Makefile与fs/compat_ioctl.c
$ vim Makefile
-KBUILD_CFLAGS  += -g
+KBUILD_CFLAGS  += -g -O0
$ vim fs/compat_ioctl.c
-       BUILD_BUG_ON(max >= (1 << 16));
+       // BUILD_BUG_ON(max >= (1 << 16));

## 删除CONFIG_CFG80211
$ make menuconfig
[*] Networking support  --->
	-*-   Wireless  --->
		< >   cfg80211 - wireless configuration API
```
