> 此文档属于文档类

### 1. markdown文件转换成pdf文件

[mdout](https://github.com/JabinGP/mdout)

```bash
$ tar -xvzf mdout.linux.x86-64.tar.gz
$ mv mdout /usr/local/bin
$ mdout install

$ mdout *.md
$ ls
*.md *.pdf
```

### 2. 将markdown文件创建为书籍

[mdbook](https://mdbook.budshome.com/)

```bash
$ cargo install mdbook

$ mdbook init
$ mdbook serve
```

[gitbook](https://github.com/GitbookIO/gitbook/blob/master/docs/setup.md)

```bash
$ npm install gitbook-cli -g

$ gitbook init
$ gitbook serve
```

