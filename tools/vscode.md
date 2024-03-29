此文章主要介绍各种`vscode`插件

### clangd

`vscode` `C/C++` 插件默认可以浏览`C`源码，但是浏览一些大型开源软件会很卡(如`linux kernel`)，
使用 `clangd` 插件能够顺畅浏览大型开源软件(如`linux kernel`)

首先，安装 clangd 软件，默认会自动下载。如果没有自动下载，执行如下命令手动下载：

```bash
$ sudo apt install clangd
```

然后，生成 `compile_commands.json`

A. 在`linux4.19 or laster`中，`linux kernel`自带可以生成`compile_commands.json`的`python`脚本

```bash
## based linux5.4
$ cd linux
$ make ARCH=x86 x86_64_defconfig
$ make ARCH=x86                     ## 编译生成bzImage以及autoconf.h
$ ./scripts/gen_compile_commands.py ## 生成compile_commands.json
```

B. 旧版本的`linux kernel`没有自带可以生成`compile_commands.json`的`python`脚本，
比如`linux2.6.34`，我们可以使用`compiledb`命令进行生成

更多关于`compiledb`详细解释，请看[官方](https://github.com/nickdiego/compiledb)

```bash
## based linux2.6.34
$ cd linux
$ make ARCH=x86 x86_64_defconfig
$ compiledb -n --command-style make ARCH=x86 ## 生成compile_commands.json
$ make ARCH=x86                              ## 编译生成bzImage以及autoconf.h
```

最后，打开`vscode`，直接对函数调用和结构体进行跳转以及自动补全

> 在浏览 linux kernel 源码时，x86_64 架构的 compile_commands.json 能够直接被 clangd 解释，
> 但是 arm64 架构的 compile_commands.json 无法被  clangd 解释，因为 clangd 无法解释 `-mabi=lp64` 标志，
> 所以我们需要为 arm64 架构添加一些额外的配置，如下:

```bash
## Linux directory
$ cat .clangd
CompileFlags:
	Remove: -mabi=lp64
```

### Native Debug

通过命令行`gdb`进行调试，比较不方便，`Native Debug`插件可以让`vscode`通过图形化进行调试

首先，在`.vscode/launch.json`进行配置，如下:

```
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "gdb",
            "request": "attach",
            "name": "gdb linux",
            "executable": "vmlinux",
            "target": "localhost:1234",
            "remote": true,
            "cwd": "${workspaceRoot}",
            "valuesFormatting": "parseText"
        }
    ]
}
```

然后，在`start_kernel()`设置断点，接着进入调试模式，最后通过F5（continue）、F10（next）、F11（step）进行源码级调试。同时在`DEBUG CONSOLE`窗口中，可以执行`gdb`命令，如`b`, `c`, `n`, `lx-version`, `lx-dmesg`等等, 如图所示:

![vscode_gdb](../resources/picture/vscode_gdb.png)

