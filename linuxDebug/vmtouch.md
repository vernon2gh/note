# 简介

vmtouch 是一个可以操纵 page cache 的工具，它可以查看一个文件占用了多少 page cache，
可以让一个文件完全浸泡在 page cache 里，也可以让一个文件完全从 page cache 中剥离
出来。

# 安装

```bash
$ apt install vmtouch
```

# 使用

```bash
$ vmtouch    [filename] ## 查询文件内容占用 pagecahe 大小
$ vmtouch -t [filename] ## 将文件内容放到 pagecahe 中
$ vmtouch -e [filename] ## 将文件内容从 pagecahe 中驱逐
```
