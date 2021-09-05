# 在6.S081使用的工具

对于这个课程，你需要一些Risc-V版本的工具：QEMU 5.1, GDB 8.3, GCC, Binutils.

### 通过APT安装（Debian/Ubuntu）

确保你运行的`debian`版本是`bullseye`或`sid`（`ubuntu`能够通过运行`cat /etc/debian_version`来检查），然后执行：

```shell
$ sudo apt-get install git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu 
```

修复`qemu-system-misc`

`qemu-system-misc`似乎更新后，破坏我们的内核的兼容性。如果我们运行`make qemu`后，系统`hand`了，你需要去卸载它，然后安装旧版本。	

```shell
$ sudo apt-get remove qemu-system-misc
$ sudo apt-get install qemu-system-misc=1:4.2-3ubuntu6
```

### 其它Linux发行版（如：编译工具链）

我们是假设安装工具链到`/usr/local`，你编译工具链时需要有一些磁盘空间（大概9GiB）

首先，克隆`RISC-V GNU Compiler`工具链仓库

```shell
$ git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
```

接着，确保你有编译工具链的相关依赖包

```shell
$ sudo apt-get install autoconf automake autotools-dev curl libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev
```

配置和编译工具链

```shell
$ cd riscv-gnu-toolchain
$ ./configure --prefix=/usr/local
$ sudo make
$ cd ..
```

接着，解压缩`QEMU 5.1.0`源码

```shell
$ wget https://download.qemu.org/qemu-5.1.0.tar.xz
$ tar xf qemu-5.1.0.tar.xz
```

编译`riscv64-softmmu`的`QEMU`

```shell
$ cd qemu-5.1.0
$ ./configure --disable-kvm --disable-werror --prefix=/usr/local --target-list="riscv64-softmmu"
$ make
$ sudo make install
$ cd ..
```

### 测试

进行测试你的安装是否成功，你运行如下命令并检查

```shell
$ riscv64-unknown-elf-gcc --version
riscv64-unknown-elf-gcc (GCC) 10.1.0
...

$ qemu-system-riscv64 --version
QEMU emulator version 5.1.0
```

你也能够编译与运行`xv6`

```shell
# in the xv6 directory
$ make qemu
# ... lots of output ...
init: starting sh
$
```

按`Ctrl-a x`执行退出操作