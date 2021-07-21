## 0. 现象

```bash
[   25.141025] icm20608 spi2.0: icm20608_probe: 
[   30.971423] icm20608 spi2.0: icm20608_open: 
[   30.971490] icm20608 spi2.0: icm20608_write_regs: 
[   30.971617] Unable to handle kernel paging request at virtual address 30393072
[   30.982512] pgd = 94920000
[   30.985315] [30393072] *pgd=00000000
[   30.990928] Internal error: Oops: 5 [#1] PREEMPT SMP ARM
[   30.996317] Modules linked in: spi_slave_icm20608
[   31.001189] CPU: 0 PID: 247 Comm: icm20608_drv_te Not tainted 4.9.88-g6fb916f7c6af-dirty #1
[   31.009599] Hardware name: Freescale i.MX6 UltraLite (Device Tree)
[   31.015845] task: 94ac5140 task.stack: 9494a000
[   31.020467] PC is at clk_core_prepare+0x1c/0x1f8
[   31.025164] LR is at clk_prepare+0x2c/0x3c
[   31.029334] pc : [<80512434>]    lr : [<80512d68>]    psr: 200e0013
[   31.029334] sp : 9494b8e8  ip : 9494b908  fp : 9494b904
[   31.040882] r10: 9494bbf7  r9 : 94497dd0  r8 : 94497a98
[   31.046172] r7 : 00000000  r6 : 94501700  r5 : 94497b30  r4 : 30393032
[   31.052763] r3 : 94ac5140  r2 : 00000001  r1 : 00000000  r0 : 30393032
[   31.059358] Flags: nzCv  IRQs on  FIQs on  Mode SVC_32  ISA ARM  Segment none
[   31.066559] Control: 10c53c7d  Table: 9492006a  DAC: 00000051
[   31.072369] Process icm20608_drv_te (pid: 247, stack limit = 0x9494a210)
[   31.079137] Stack: (0x9494b8e8 to 0x9494c000)
[   31.083577] b8e0:                   30393032 94497b30 94501700 00000000 9494b91c 9494b908
...
[   31.546273] bfe0: 00000000 7e828c34 000104c7 76e98c46 000e0030 000105d0 00000000 00000000
[   31.554585] [<80512434>] (clk_core_prepare) from [<80512d68>] (clk_prepare+0x2c/0x3c)
[   31.562528] [<80512d68>] (clk_prepare) from [<80684370>] (spi_imx_prepare_message+0x24/0x9c)
[   31.571073] [<80684370>] (spi_imx_prepare_message) from [<8067f29c>] (__spi_pump_messages+0x234/0x758)
[   31.580486] [<8067f29c>] (__spi_pump_messages) from [<8067fa3c>] (__spi_sync+0x258/0x25c)
[   31.588766] [<8067fa3c>] (__spi_sync) from [<8067fa74>] (spi_sync+0x34/0x4c)
[   31.595947] [<8067fa74>] (spi_sync) from [<7f0002c8>] (icm20608_write_regs+0xf8/0x198 [spi_slave_icm20608])
[   31.605836] [<7f0002c8>] (icm20608_write_regs [spi_slave_icm20608]) from [<7f0003f4>] (icm20608_open+0x8c/0x1f4 [spi_slave_icm20608])
[   31.617969] [<7f0003f4>] (icm20608_open [spi_slave_icm20608]) from [<8026edfc>] (chrdev_open+0xb4/0x198)
[   31.627555] [<8026edfc>] (chrdev_open) from [<8026790c>] (do_dentry_open.constprop.3+0x1f4/0x31c)
[   31.636526] [<8026790c>] (do_dentry_open.constprop.3) from [<8026890c>] (vfs_open+0x50/0x80)
[   31.645062] [<8026890c>] (vfs_open) from [<80278c18>] (path_openat+0x3d0/0xf44)
[   31.652472] [<80278c18>] (path_openat) from [<8027a720>] (do_filp_open+0x74/0xd8)
[   31.660055] [<8027a720>] (do_filp_open) from [<80268ce8>] (do_sys_open+0x124/0x1d4)
[   31.667806] [<80268ce8>] (do_sys_open) from [<80268dc0>] (SyS_open+0x28/0x2c)
[   31.675044] [<80268dc0>] (SyS_open) from [<80109100>] (ret_fast_syscall+0x0/0x1c)
[   31.682619] Code: e52de004 e8bd4000 e2504000 0a00001e (e5943040) 
[   31.700966] ---[ end trace 29fded49d5ea7f40 ]---
```

从上述可知，kernel出现Oops，内容是Unable to handle kernel paging request at virtual address 30393072

通过dmesg可知是因为icm20608_write_regs()函数调用spi_sync()函数

通过stack call可知，最后会调用clk_core_prepare()函数（此函数在driver/clk/clk.c实现）

并且可得到PC，LR，如下

```bash
PC is at clk_core_prepare+0x1c/0x1f8   
LR is at clk_prepare+0x2c/0x3c         
pc : [<80512434>]    lr : [<80512d68>]    psr: 200e0013

# PC在clk_core_prepare()函数偏移0x1c位置，即0x80512434
# LR在clk_prepare()函数偏移0x2c位置,即0x80512d68
```

## 1. 反汇编定位源码

首先，通过`make menuconfig`开启kernel调试信息选项，如下

```bash
diff --git a/arch/arm/configs/imx_v7_alpha_defconfig b/arch/arm/configs/imx_v7_alpha_defconfig
index d96432952bea..128a88e37344 100644
--- a/arch/arm/configs/imx_v7_alpha_defconfig
+++ b/arch/arm/configs/imx_v7_alpha_defconfig
@@ -4485,7 +4485,11 @@ CONFIG_DYNAMIC_DEBUG=y
 #
 # Compile-time checks and compiler options
 #
-# CONFIG_DEBUG_INFO is not set
+CONFIG_DEBUG_INFO=y
+# CONFIG_DEBUG_INFO_REDUCED is not set
+# CONFIG_DEBUG_INFO_SPLIT is not set
+# CONFIG_DEBUG_INFO_DWARF4 is not set
+# CONFIG_GDB_SCRIPTS is not set
 CONFIG_ENABLE_WARN_DEPRECATED=y
 CONFIG_ENABLE_MUST_CHECK=y
 CONFIG_FRAME_WARN=1024
```

然后，通过`make zImage`重新编译kernel，产生vmlinux文件与clk.o文件

最后，反汇编vmlinux文件或clk.o文件

```bash
# 其中-S选项，将源码与汇编同时显示
$ arm-linux-gnueabihf-objdump -S vmlinux > vmlinux.dump
或
$ arm-linux-gnueabihf-objdump -S clk.o > clk.dump
```

## 2. 分析

**PC在clk_core_prepare()函数偏移0x1c位置，即0x80512434**

可以通过vmlinux.dump分析，也可以通过clk.dump分析，也可以通过 addr2line 或 gdb 分析，四选一即可

### 2.1 通过vmlinux.dump分析

在vmlinux.dump中查找0x80512434即可

```c
80512418 <clk_core_prepare>:                                
{                                                         
80512418:|  e1a0c00d |  mov|ip, sp                                               
8051241c:|  e92dd8f0 |  push|   {r4, r5, r6, r7, fp, ip, lr, pc}                 
80512420:|  e24cb004 |  sub|fp, ip, #4                                         
80512424:|  e52de004 |  push|   {lr}|   |   ; (str lr, [sp, #-4]!)               
80512428:|  ebeffd6e |  bl| 801119e8 <__gnu_mcount_nc>                           
|   if (!core)                                                         
8051242c:|  e2504000 |  subs|   r4, r0, #0                                
80512430:|  0a00001e |  beq|805124b0 <clk_core_prepare+0x98>                     
|   if (core->prepare_count == 0) {                                               
80512434:|  e5943040 |  ldr|r3, [r4, #64]|  ; 0x40    # 读core->prepare_count值     
80512438:|  e3530000 |  cmp|r3, #0                    # 将core->prepare_count与0进行比较 
8051243c:|  0a000004 |  beq|80512454 <clk_core_prepare+0x3c>                     
|   return 0;                                                            
80512440:|  e3a05000 |  mov|r5, #0
    ....
}
```

可知，读core->prepare_count值时，出现Oops

### 2.2 通过clk.dump分析

在clk.dump中查找clk_core_prepare函数

```c
000029a8 <clk_core_prepare>:                                
{                                                                           
    29a8:|  e1a0c00d |  mov|ip, sp                                
    29ac:|  e92dd8f0 |  push|   {r4, r5, r6, r7, fp, ip, lr, pc}                     
    29b0:|  e24cb004 |  sub|fp, ip, #4                                
    29b4:|  e52de004 |  push|   {lr}|   |   ; (str lr, [sp, #-4]!)                   
    29b8:|  ebfffffe |  bl| 0 <__gnu_mcount_nc>                                     
|   if (!core)                                
    29bc:|  e2504000 |  subs|   r4, r0, #0                                
    29c0:|  0a00001e |  beq|2a40 <clk_core_prepare+0x98>                             
|   if (core->prepare_count == 0) {                                   
    29c4:|  e5943040 |  ldr|r3, [r4, #64]|  ; 0x40  # 读core->prepare_count值             
    29c8:|  e3530000 |  cmp|r3, #0                  # 将core->prepare_count与0进行比较   
    29cc:|  0a000004 |  beq|29e4 <clk_core_prepare+0x3c>                             
|   return 0;                                                       
    29d0:|  e3a05000 |  mov|r5, #0 
	....  
}
```

clk_core_prepare函数地址是0x000029a8，0x000029a8 + 0x1c = 0x000029c4

可知，读core->prepare_count值时，出现Oops

### 2.3 通过addr2line分析

只要提供出错的PC绝对地址给addr2line，就可以直接得到哪一个文件以及哪一行源码出错

```bash
$ xxx-addr2line -e vmlinux 0x80512434
```

### 2.4 通过gdb分析

只要提供出错的PC相对地址给gdb，就可以直接得到哪一个文件以及哪一行源码出错

```bash
$ xxx-gdb vmlinux
(gdb) list *(clk_core_prepare+0x1c)
```

## 3. 解决问题

通过分析后，可知 读core->prepare_count值时，出现Oops。

为什么？

linux clk子系统一般会调用clk_prepare_enable() 进行准备+使能对应clk，此处要准备+使能的clk是spi clk

通过stack call可知，已经通过spi_imx_prepare_message()函数调用clk_prepare_enable() 。



那为什么还是Oops尼？但是问题肯定是**spi clk没有准备+使能**

通过`dmesg | grep spi`可知，如下

```bash
spi_imx 2010000.ecspi: No CS GPIOs available
spi_imx: probe of 2010000.ecspi failed with error -22
```

可知，**spi总线没有初始化成功**



通过阅读driver/spi/spi-imx.c源码后，需要做如下修改：

```c
diff --git a/drivers/spi/spi-imx.c b/drivers/spi/spi-imx.c
index 284a0de26fd8..d0344a3a1e81 100644
--- a/drivers/spi/spi-imx.c
+++ b/drivers/spi/spi-imx.c
@@ -1290,7 +1290,7 @@ static int spi_imx_probe(struct platform_device *pdev)
                dev_err(&pdev->dev, "bitbang start failed with %d\n", ret);
                goto out_clk_put;
        }
-
+/*
        if (!master->cs_gpios) {
                dev_err(&pdev->dev, "No CS GPIOs available\n");
                ret = -EINVAL;
@@ -1309,7 +1309,7 @@ static int spi_imx_probe(struct platform_device *pdev)
                        goto out_clk_put;
                }
        }
-
+*/
        dev_info(&pdev->dev, "probed\n");
 
        clk_disable_unprepare(spi_imx->clk_ipg);
```

修改后，重新编译，烧写zImage后，通过`dmesg | grep spi`如下，即已经成功初始化spi总线

```bash
spi_imx 2010000.ecspi: probed
```

重新加载spi icm20608驱动模块，已经可以正常读写icm20608设备

