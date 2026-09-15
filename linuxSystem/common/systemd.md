## 简介

`systemd` 是 Linux 系统中广泛使用的**初始化系统（init）和服务管理器**。它作为
PID 1 启动，负责：

- 系统启动时拉起各种服务、挂载文件系统、配置网络等；
- 用 **unit（单元）** 统一管理服务、套接字、定时器、挂载点、设备等；
- 为每个 unit 创建独立的 **cgroup**，因此可以方便地做资源限制（CPU、内存、IO 等）；
- 提供日志系统 `journald`，配合 `journalctl` 查看日志。

systemd, systemctl, systemd-run 的关系与区别：

- **systemd**：后台守护进程，真正管理系统和 cgroup；
- **systemctl**：日常管理命令，控制持久化或已存在的 unit；
- **systemd-run**：临时创建 unit 并运行命令，常用于测试和资源限制。

简单说：`systemd` 是管家，`systemctl` 是遥控器，`systemd-run` 是临时开一个
受控房间跑命令。

## systemctl

`systemctl` 是与 systemd 交互的主要命令行工具，用来**管理 unit 的生命周期和状态**。

常用命令：

```bash
systemctl enable nginx         # 设置开机自启
systemctl disable nginx        # 取消开机自启
systemctl start nginx          # 启动服务
systemctl stop nginx           # 停止服务
systemctl restart nginx        # 重启
systemctl status nginx         # 查看状态
```

## systemd-run

`systemd-run` 用于**临时创建一个 transient unit**，创建临时 cgroup 并运行指定命令。

它不会像普通服务那样永久写入 unit 文件，命令结束后 unit 通常自动清理。
非常适合测试、一次性任务、资源限制实验。

```bash
$ systemd-run --scope /path/to/your/test_program
```

以上操作，类似手动创建 cgroup：

```bash
$ mkdir /sys/fs/cgroup/mytest
$ echo $$ > /sys/fs/cgroup/mytest/cgroup.procs   ## 将当前 Shell PID 加入该 cgroup
$ /path/to/your/test_program                     ## 在该 Shell 中执行测试程序
$ rmdir /sys/fs/cgroup/mytest
```

也可以通过 systemd 服务文件持久化配置

```bash
$ cat /etc/systemd/system/mytest.service
[Unit]
Description=My Test Service

[Service]
ExecStart=/path/to/your/test_program

[Install]
WantedBy=multi-user.target

$ systemctl start mytest
```

可选参数：

* `--scope`         ：创建临时 scope 单元，命令退出后 cgroup 会自动清理。
* `--user`          ：运行在 user unit, 默认是 system unit。
* `-p MemoryHigh=2G`：将 cgroup `memory.high` 设置为 2GB，其他所有属性都在
                    `man systemd.resource-control`手册页中有详细说明
