> 在内核开发过程中，经常需要对二进制文件进行查看/编辑/对比操作

### hexdump

* 简介

hexdump是shell 二进制文件查看器，允许用户查看二进制文件。

* 安装

```bash
$ sudo apt install hexdump
```

* 使用

```bash
$ hexdump -C <filename>
```

### hexedit

* 简介

hexedit是shell 二进制文件编辑器，允许用户查看/编辑二进制文件。

* 安装

```bash
$ sudo apt install hexedit
```

* 使用

```bash
$ hexedit <filename>
```

### ghex

* 简介

[GHex](https://wiki.gnome.org/Apps/Ghex)是GUI 二进制文件编辑器，允许用户查看/编辑二进制文件。

* 安装

```bash
$ sudo apt install ghex
```

* 使用

```bash
$ ghex <filename>
```

### HexEditor

* 简介

HexEditor是vscode 二进制文件编辑器，允许用户查看/编辑二进制文件。

* 安装

  直接在vscode Extensions进行查找 HexEditor ，点击 install 即可

* 使用

  Right click a file --> Open with... --> Hex Editor

### hexcompare

* 简介

hexcompare是shell 二进制文件对比器，允许用户对比二进制文件的区别。

* 安装

```bash
$ sudo apt install hexcompare
```

* 使用

```bash
$ hexcompare <filename1> <filename2>
```

