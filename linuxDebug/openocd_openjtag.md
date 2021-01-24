### 前提
* 准备openjtag调试器
* 准备S3C2440开发板，本文以jz2440为例
* 连接: jz2440 JTAG -- openjtag调试器 -- PC机USB

### 安装openocd
```bash
$ sudo apt install openocd
```

### 运行openocd
```bash
$ openocd -f interface/ftdi/100ask-openjtag.cfg -f board/mini2440.cfg

参考：
$ openocd -f interface/openjtag.cfg -f target/samsung_s3c2440.cfg
```

### 基于openocd烧写裸机/u-boot以及openocd调试命令

* 进入openocd命令行

```bash
$ telnet localhost 4444
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Open On-Chip Debugger
> help
```

* openocd常用命令
** 附件 表１**


* 使用 OpenOCD 下载裸机小程序到内部 RAM，并运行、调试
```bash
$ telnet localhost 4444
# 下载裸机小程序到内部 RAM, 并运行
> halt
> load_image /xxx/xxx/leds_elf 0x0 elf	# 下载裸机小程序到内部 RAM
> resume 0x0

# 调试
> halt
> bp 0x70 4 hw	# 在地址0x70设置硬件/hw断点
breakpoint set at 0x00000070
> bp			# 查看断点
Breakpoint(IVA): 0x00000070, 0x4, 1
> resume		# 运行一次，中断一次

> bp
Breakpoint(IVA): 0x00000070, 0x4, 1
> rbp 0x70		# 删除断点
> bp
> resume 		# 一直运行，不会中断
```

* 使用 OpenOCD 下载 u-boot，通过 u-boot 烧写 Nor/Nand Flash
```

```

### 基于arm-linux-gdb调试 裸机/u-boot以及linux内核
* 前提
1) 程序必须位于链接地址
	** u-boot 链接地址是SDRAM/DDR，需要先初始化SDRAM/DDR **
	** linux内核 链接地址是虚拟地址，需要先初始化MMU **
2) 链接脚本必须有.text、.rodata、.data、.bss段
3) 程序必须有调试信息，即编译程序时加 -g 参数


* gdb常用命令
** 附件 表2 **


* 调试裸机
```bash
$ telnet localhost 4444 
> halt


$ arm-linux-gdb leds_elf 
(gdb) target remote localhost:3333
(gdb) load
```

* 调试u-boot
```bash
$ telnet localhost 4444 
> halt
> init_2440


$  arm-linux-gdb u-boot
(gdb) target remote localhost:3333
(gdb) load
```

* 调试linux内核
```bash
# 通过u-boot加载linux内核到内存0x32000000中
OpenJTAG> nand read 0x32000000 0x60000 0x200000
或
OpenJTAG> tftp 0x32000000 uImage


$ telnet localhost 4444 
> halt
> bp 0x30008060 4 hw	# 在__turn_mmu_on打上断点，通过System.map可知
> resume


# 启动linux内核
OpenJTAG> bootm 0x32000000

uImage 的开始有个自解压的程序,它把真正的内核解压出来放在 0x30008000 开始的内存,
解压完后再跳到 0x30008000 去执行真正的内核


$ telnet localhost 4444 
> resume	# linux内核自解压过程中停止，继续执行
> step	# 停止在__turn_mmu_on处，单步执行，观察MMU是否使能
target halted in ARM state due to single-step, current mode: Supervisor
cpsr: 0x200000d3 pc: 0x30008064
MMU: disabled, D-Cache: disabled, I-Cache: enabled
> step	# 单步执行，观察MMU是否使能
target halted in ARM state due to single-step, current mode: Supervisor
cpsr: 0x200000d3 pc: 0x30008068
MMU: enabled, D-Cache: enabled, I-Cache: enabled	# 此处MMU已使能
> rbp 0x30008060  # 删除openocd硬件断点，然后进入真正linux内核源码级调试


$ arm-linux-gdb vmlinux 
(gdb) target remote localhost:3333
(gdb) b start_kernel
Breakpoint 1 at 0xc00086cc: file init/main.c, line 502.
(gdb) c
```


### 基于eclipse调试 裸机/u-boot以及linux内核
```

```



**参考连接：**
https://zhuanlan.zhihu.com/p/41517198
https://elinux.org/OpenOCD

**附件**

* 表１

| openocd常用命令                                        |						|
| ----------------------------------------------------- | -------------------------------- |
| poll                                                             | 查询目标板当前状态            |
| halt                                                             | 中断目标板的运行               |
| resume [address]                                       | 恢复目标板的运行               |
| step [address]                                            | 单步执行                            |
| reset                                                           | 复位目标板                         |
| bp &lt;addr&gt;  &lt;length&gt; [hw]         | 在地址addr处设置断点，指令长度为length，hw表示硬件断点 |
| rbp &lt;addr&gt;                                         | 删除地址addr处的断点                                                          |
| mdw [phys] &lt;addr&gt; [count]              | 显示从(物理)地址 addr 开始的 count(缺省是 1)个字(4 字节)    |
| mww [phys] &lt;addr&gt; &lt;value&gt;   | 向(物理)地址 addr 写入一个字,值为 value                               |
| load_image &lt;file&gt; &lt;address&gt; [bin/ihex/elf] | 将文件file载入地址为 address 的内存,格式有bin、ihex、elf                 |
| dump_image &lt;file&gt; &lt;address&gt; &lt;size&gt; | 将内存从地址 address 开始的 size 字节数据读出,保存到文件file中        |
| verify_image &lt;file&gt; &lt;address&gt; [bin/ihex/elf] | 将文件file与内存 address 开始的数据进行比较,格式有bin、ihex、elf |
| reg                                                              | 打印寄存器的值                     |
| virt2phys &lt;virtual_address&gt;             | 获得虚拟地址对应的物理地址  |
| script &lt;file&gt;                                       | 执行 file 文件中的命令            |


* 表２

| gdb常用命令                                      |                                                              |
| --------------------------------------------- | ------------------------------------------------ |
| target remote &lt;ip:port&gt;           | 远程连接                                                |
| quit                                                   | 退出GDB                                                |
| file &lt;FILE&gt;                                | 载入文件 FILE，注意 : 不会下载到目标板  |
| load [FILE] | 把文件下载到目标板，如果不指定FILE,则下载之前指定过的(比如 file 命令指定的,或是 gdb 运行时指定的文件) |
| list                                                    | 查看源码                                                 |
| break *&lt;address&gt;                    | 在某个地址上设置断点,比如 break *0x84 |
| info source                                       | 查看当前源程序                                        |
| info stack                                         | 查看堆栈信息                                           |
| info args                                           | 查看当前的参数                                        |
| break &lt;FUNCTION&gt;                 | 在函数入口设置断点                                 |
| break &lt;FILENAME:LINENUM&gt; | 在指定源文件的某一行上设置断点              |
| info break                                        | 查看断点                                                  |
| delete &lt;number&gt;                    | 删除断点                                                  |
| watch &lt;EXPRESSION&gt;             | 当指定变量被写时,程序被停止                   |
| rwatch &lt;EXPRESSION&gt;            | 当指定变量被读时,程序被停止                   |
| print &lt;EXPRESSION&gt;               | 查看变量                                                  |
| set varible=value                           | 设置变量                                                  |
| step                                                 | 单步执行，会跟踪进入一个函数                   |
| next                                                 | 单步执行，不会进入函数                              |
| continue                                          | 继续执行程序，加载程序后也可以用来启动程序 |
| help [command]                             | 列出帮助信息或是列出某个命令的帮助信息 |
| monitor &lt;command ...&gt;         | 调用 gdb 服务器软件的命令，比如:“monitor mdw 0x0” 调用 openocd 本身的命令“mdw 0x0” |


