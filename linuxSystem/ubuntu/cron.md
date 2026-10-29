## 简介

在 Ubuntu 中创建定时任务通常使用 cron 服务。
cron 在后台运行，默认不会输出任何内容到终端上。

## 步骤

在 crontab 中配置调用脚本，如下：

```
$ crontab -e
0 9 * * * /path/to/scripts.sh
```

在每天早上 9 点自动执行 `scripts.sh` 脚本
