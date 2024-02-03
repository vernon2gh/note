# 简介

top/htop 显示系统资源的占用情况

# 使用

```bash
$ top               ## 以进程为单位显示系统资源的占用情况
$ top -H            ## 以线程为单位显示系统资源的占用情况
$ htop              ## 相当于 top -H，并且界面更加好看

$ top -H -p <pid>   ## 只显示 pid 进程的情况
$ htop -p <pid>     ## 只显示 pid 进程的情况
```
