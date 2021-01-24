### Ubuntu中如何获取某个命令的源代码

```
$ apt source <package_name>

## example following:
## 1. 确定命令来自哪一个文件
$ which ls
/bin/ls

## 2. 确定这个文件来自哪一个包
$ dpkg -S /bin/ls
coreutils: /bin/ls

## 3. 下载源码到当前目录。
$ apt source coreutils
```

