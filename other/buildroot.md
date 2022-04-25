### 1. Building out-of-tree

```bash
$ make O=output/xxx xxx_defconfig # 配置buildroot
$ cd output/xxx
$ make <target>
```

###  2. how to rebuild packages

```bash
$ cd output/xxx

$ make menuconfig
$ make <package>-dirclean    # 完全删除
$ make <package>-reconfigure # 重新配置、编译、安装
$ make <package>-rebuild     # 重新编译、安装
$ make <package>-reinstall   # 重新安装
```

### 3. linux kernel 无法引导 buildroot编译的rootfs

1. 检查是否将rootfs镜像存放在指定的块设备中
2. buildroot编译的rootfs类型(ext2, ext3 and ext4 etc) 与 kernel支持的rootfs类型 是否相同
3. buildroot编译的rootfs /linuxrc 是否成功执行

### 4. 添加本地源码包到buildroot

先编写buildroot package框架：

```bash
$ vim package/Config.in
source "package/helloworld/Config.in"

$ vim package/helloworld/Config.in
config BR2_PACKAGE_HELLOWORLD
	bool "helloworld"
	help
	  Select the helloworld.

$ vim package/helloworld/helloworld.mk
HELLOWORLD_VERSION = 20200622
HELLOWORLD_SITE = $(TOPDIR)/localcode/helloworld  # 本地源码的位置
HELLOWORLD_SITE_METHOD = local                    # 使用本地源码

define HELLOWORLD_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all # 编译命令
endef

define HELLOWORLD_INSTALL_TARGET_CMDS
	$(MAKE) -C $(@D) install                      # 安装命令
endef

$(eval $(generic-package))

$ make menuconfig
Target packages  --->
	[*] helloworld
```

然后编写源码：

```bash
$ vim localcode/helloworld/helloworld.c
#include <stdio.h>

int main(void)
{
	printf("hello world, vernon\n");
	return 0;
}

$ vim localcode/helloworld/Makefile
all : helloworld

helloworld :
	$(TARGET_CROSS)gcc helloworld.c -o helloworld       # 编译命令

install :
	install -D -m 0755 helloworld $(TARGET_DIR)/usr/bin # 安装命令
```

### 参考网址

[buildroot手册](https://buildroot.org/downloads/manual/manual.html)
