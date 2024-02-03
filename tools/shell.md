### 0. Shell

* zsh（`.zshrc`）

使用 [oh-my-zsh](https://github.com/ohmyzsh/ohmyzsh) 更加容易配置 zsh，
如：主题、历史命令自动提示
[zsh-autosuggestions](https://github.com/zsh-users/zsh-autosuggestions) 等

* bash（`.bashrc`）

使用默认配置即可。

### 1. 怎么设置PATH

以在PATH中添加/home/book目录为例：

- 永久设置，对所有用户都有效：

修改/etc/environment

```
PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/home/book"
```

然后重启系统或重新登录。

- 永久设置，只对当前用户有效：

修改~/.bashrc

```
export PATH=$PATH:/home/book
```

然后重启系统或重新登录。

- 临时设置：

在终端执行以下命令，这只对当前终端有效：

```
export PATH=$PATH:/home/book
```

### 2. 帮助信息man/info/help命令

Linux系统中提供了**三种**帮助方式。

`man`和`info`是独立的命令， `--help`是个命令的参数， 它们都是Linux中获取帮助信息最权威，最快捷的途径。

* man使用的最多

```
　man man　　//查看man手册的说明
　man ls　　　//当没有指定使用那一页，默认使用第1页
　man 1 ls　　//与　man ls 一样
　man 1 gcc　//gcc是一个应用程序，在linux中一般使用gcc编译器来编译c/c++语言的程序　
　man 2 open　//查看系统调用open的man手册说明。open/write/read/close等等都是系统调用
```

**注意：man手册的9册内容的侧重点，最好记一下**

| section | 名称                     | 说明                              |
| ------- | ------------------------ | --------------------------------- |
| 1       | 可执行程序或shell命令    | 用户可操作的命令                  |
| 2       | 系统调用                 | 内核提供的函数(查头文件)          |
| 3       | 库调用                   | 常用的函数库                      |
| 4       | 特殊文件                 | 在/dev下的设备文件                |
| 5       | 文件格式和约定           | 对一些文件进行解释，如/etc/passpd |
| 6       | 游戏程序                 | 游戏程序                          |
| 7       | 杂项                     | 包括宏包和约定等                  |
| 8       | 系统管理员使用的管理命令 | 通常只有系统管理员root可以使用    |
| 9       | 内核相关                 | Linux内核相关文件                 |

* info

```
info ls　　//查看ls的帮助信息
```

* --help

```
ls --help　//查看ls的帮助信息
```

### 3. find命令

* 目的：查找符合条件的文件
* 格式：
```
find [目录名] [选项] [查找条件]
```

在/work/001_linux_basic/dira/目录中查找文件名为test1.txt的文件

```
find /work/001_linux_basic/dira/  -name "test1.txt"
```

### 4. grep命令

* 目的：使用grep命令来查找文件中符合条件的字符串
* 格式：

```
grep -rni "字符串" [文件名]
```

`r`(recursive)：递归查找

`n`(number)：显示目标位置的行号

`i`(ignore)：忽略大小写

字符串：要查找的字符串

文件名：要查找的目标文件，如果是*则表示查找当前目录下的所有文件和目录


 举例：

`grep -n "abc" test1.txt`  在test1.txt中查找字符串abc

`grep -rn "abc" * `       在当前目录递归查找字符串abc

**注意：可以加入-w全字匹配**

### 5. file命令

* 目的：识别文件类型
* 格式：
```
file [文件名]
```

在Linux中有一个非常重要的观点：**Linux下一切皆文件**。

举例：

```
file ~/.bashrc      为ASCII 编码的text类型
file ~/.vimrc       为UTF-8 Unicode 编码的text类型
file ~/Pictures/*   如图形文件JPEG/PNG/BMP格式
file ~/100ask/      为directory表明这是一个目录
file /bin/pwd       出现 ELF 64-bit LSB executable，即为ELF格式的可执行文件
file /dev/*         出现character special(字符设备文件)、 block special(块设备文件)等
```

### 6. which/whereis命令

* 目的：查找命令或应用程序的所在位置
* 格式：
```
which [命令名/应用程序名]
```


 在终端上执行`pwd`实际上是去执行了`/bin/pwd`


 举例：

```
which pwd     定位到/bin/pwd
which gcc     定位到/usr/bin/gcc
whereis pwd   查找到可执行程序的位置/bin/pwd和手册页的位置/usr/share/man/man1/pwd.1.gz
```

### 7. gzip命令和bzip2命令

**压缩的概念**

* 压缩的目的：

在网络传递文件时，可以先将文件压缩，然后传递压缩后的文件，从而减少网络带宽。接受者接受文件后，解压即可。

* 压缩的类型

有损压缩、无损压缩。

> a)有损压缩： 如mp4视频文件，即使压缩过程中，减少了很多帧的数据， 对观看者而言，也没有影响。当然mp3音乐文件也是有损压缩。
>
> b)无损压缩： 如普通文件的压缩，为了保证信息的正确传递， 不希望文件经过压缩或解压后，出现问题。 后面讲解的都是无损压缩。

* 单个文件的压缩(解压)使用gzip和bzip2

* 多个文件和目录使用tar



* gzip的常用选项：

```
-l(list)	列出压缩文件的内容
-k(keep)	在压缩或解压时，保留输入文件。
-d(decompress)	将压缩文件进行解压缩
```

1）查看

```gzip -l [压缩文件名]```

比如： `gzip -l pwd.1.gz`

2）解压

```gzip -kd [压缩文件名]```

比如： `gzip -kd pwd.1.gz` 	该压缩文件是以.gz结尾的单个文件

3）压缩

```gzip -k [源文件名]```

比如：`gzip -k mypwd.1` 得到了一个.gz结尾的压缩文件

**注意： **

1）如果gzip不加任何选项，此时为压缩，压缩完该文件会生成后缀为.gz的压缩文件， 并删除原有的文件，所以说，推荐使用`gzip -k`来压缩源文件。

2）相同的文件内容，如果文件名不同，压缩后的大小也不同。

3）gzip只能压缩单个文件，不能压缩目录。

**提示： **

`man pwd`会解压`/usr/share/man/man1/pwd.1.gz`这个文件， 然后读取该文件中固定的格式的一些信息，然后显示到终端中。



* bzip2的常用选项:

```
-k(keep)	在压缩或解压时，保留输入文件。
-d(decompress)	将压缩文件进行解压缩
```

1）压缩

```bzip2  -k  [源文件名]```

比如：`bzip2 -k mypwd.1` 	得到一个.bz2后缀的压缩文件

2）解压

```bzip2 -kd [压缩文件名]```

比如：`bzip2 -kd mypwd.1.bz2`

**注意：**

1）如果bzip2不加任何选项，此时为压缩，压缩完该文件会生成后缀为.bz2的压缩文件， 并删除原有的文件，所以说，推荐使用`bzip2 -k`来压缩源文件。

2）bzip2只能压缩单个文件，不能压缩目录。



**单个文件的压缩使用gzip或bzip2**

压缩有两个参数：

1）压缩时间

2）压缩比


一般情况下，**小文件使用gzip来压缩，大文件使用bzip2来压缩**。


比如：

mypwd.1源大小是1477字节，

gzip压缩后mypwd.1.gz是877字节，

bzip2压缩后mypwd.1.bz2是939字节。




myls.1源文件大小7664字节，

gzip压缩后myls.1.gz是3144字节，

bzip2压缩后myls.1.bz2是3070字节。

**gzip、bizp2只能对一个文件进行压缩，而不能对多个文件和目录进行压缩。 所以需要tar来对多个目录、文件进行打包和压缩。 **



* tar常用选项

```
-c(create) ：表示创建用来生成文件包
-x         ：表示提取，从文件包中提取文件
-t         ：可以查看压缩的文件。 -z使用gzip方式进行处理，它与”c“结合就表示压缩，与”x“结合就表示解压缩。
-j         ：使用bzip2方式进行处理，它与”c“结合就表示压缩，与”x“结合就表示解压缩。
-v(verbose)：详细报告tar处理的信息
-f(file)   ：表示文件，后面接着一个文件名。
-C <指定目录>：解压到指定目录
```

**tar打包、gzip压缩**

1）压缩

```tar -czvf [压缩文件名] [目录名] ```

如： `tar czvf dira.tar.gz dira`

注意：`tar -czvf`与`tar czvf`是一样的效果，所以说，后面统一取消-。

2）查看

```tar tvf [压缩文件名]```

如：`tar tvf dira.tar.gz`


3）解压

```tar xzvf [压缩文件名]```

```tar xzvf [压缩文件名] -C [指定目录]```

如： `tar xzvf dira.tar.gz`    解压到当前目录

如： `tar xzvf dira.tar.gz -C /home/book`    解压到/home/book

**tar打包、bzip2压缩**

1）压缩

```tar cjvf [压缩文件名] [目录名]```

如： `tar cjvf dira.tar.bz2  dira`

2）查看

```tar tvf [压缩文件名]```

如： `tar tvf dira.tar.bz2`

3）解压

```tar xjvf [压缩文件名]```

```tar xjvf [压缩文件名] -C [指定目录]```

如： `tar xjvf dira.tar.bz2`    解压到当前目录

如： `tar xjvf dira.tar.bz2 -C  /home/book`   解压到/home/book

### 8. 自动输入密码

```bash
$ sudo apt install expect

$ vim test
#!/bin/expect
set timeout 30

spawn sudo su - root
expect "password:"
send "123\n"

interact

$ expect test
```

> 参考：[Shell脚本交互：自动输入密码](https://www.codeleading.com/article/4318624006/)

