> 此文档属于解决类文档，直接解决遇到的问题。

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

### 3. add arm-linux-gdb

```bash
$ cd output/xxx

$ make menuconfig
Toolchain  ---> 
[*] Build cross gdb for the host
```

### 4. add arm-linux-gdb code display

```bash
$ cd output/xxx

$ make menuconfig
Toolchain  ---> 
[*]   TUI support 
```

### 5. Incorrect selection of kernel headers: expected 2.6.x, got 3.13.x

```bash
$ make menuconfig
Toolchain  --->
        External toolchain kernel headers series (3.13.x)  --->
```

### 6. Incorrect selection of the C library

```bash
$ make menuconfig
Toolchain  --->
        External toolchain C library (glibc/eglibc)  --->
```

### 7. RPC support available in C library, please enable BR2_TOOLCHAIN_HAS_NATIVE_RPC

```bash
$ make menuconfig
Toolchain  --->
        [*] Toolchain has RPC support?
```

### 8. Incorrect selection of gcc version: expected 7.x, got 7

查找报错时执行的命令在某文件某行：

```bash
$ cd buildroot
$ grep -rni "Incorrect selection of gcc version:"
toolchain/helpers.mk:191
$ vim toolchain/helpers.mk
```

修复如下：

```bash
--- a/toolchain/helpers.mk
+++ b/toolchain/helpers.mk
@@ -184,7 +187,7 @@ check_gcc_version = \
        real_version=`$(1) -dumpversion` ; \
-       if [[ ! "$${real_version}" =~ ^$${expected_version}\. ]] ; then \
+       if [[ ! "$${real_version}" =~ ^$${expected_version} ]] ; then \
                printf "Incorrect selection of gcc version: expected %s.x, got %s\n" \
                        "$${expected_version}" "$${real_version}" ; \
                exit 1 ; \
```

### 9. package/pkg-generic.mk:360

详细报错信息如下：

```bash
>>> toolchain-external-custom  Copying external toolchain libraries to target...
package/pkg-generic.mk:360: recipe for target 'output/build/toolchain-external-custom/.stamp_target_installed' failed

(可选)显示报错时执行的命令：
$ make --trace
```

查找报错时执行的命令在某文件某行：

```bash
$ cd buildroot
$ grep -rni "Copying external toolchain libraries to target"
toolchain/toolchain-external/pkg-toolchain-external.mk:403
$ vim toolchain/toolchain-external/pkg-toolchain-external.mk +403
402 define TOOLCHAIN_EXTERNAL_INSTALL_TARGET_LIBS
403         $(Q)$(call MESSAGE,"Copying external toolchain libraries to target...")
404         $(Q)for libpattern in $(TOOLCHAIN_EXTERNAL_LIBS); do \
405                 $(call copy_toolchain_lib_root,$$libpattern); \
406         done
407 endef
$ grep -rni "copy_toolchain_lib_root"
toolchain/helpers.mk:11
$ vim toolchain/helpers.mk +11
```

修复如下：

```bash
--- a/toolchain/helpers.mk
+++ b/toolchain/helpers.mk
@@ -17,6 +17,9 @@ copy_toolchain_lib_root = \
                        mkdir -p $(TARGET_DIR)/$${DESTDIR}; \
                        rm -fr $(TARGET_DIR)/$${DESTDIR}/$${LIBNAME}; \
+                       if test "$${DESTDIR}" = "etc" ; then \
+                               break ; \
+                       fi; \
                        if test -h $${LIBPATH} ; then \
                                cp -d $${LIBPATH} $(TARGET_DIR)/$${DESTDIR}/$${LIBNAME}; \
                                LIBPATH="`readlink -f $${LIBPATH}`"; \
```

### 10. linux kernel 无法引导 buildroot编译的rootfs

1. 检查是否将rootfs镜像存放在指定的块设备中
2. buildroot编译的rootfs类型(ext2, ext3 and ext4 etc) 与 kernel支持的rootfs类型 是否相同
3. buildroot编译的rootfs /linuxrc 是否成功执行

### 11. 添加本地源码包到buildroot

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