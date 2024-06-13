## 简介

什么是内网穿透（NAT, Network Address Translation）？

简单来说就是通过某些工具打通内网与公网，让公网可以获取内网数据，这样在外地办公
人员可以在任何地方愉快的访问家里局域网的服务器。

比如：在内网服务器运行 [natapp](https://natapp.cn) 隧道之后，natapp 会分配一个
（公网）专属域名:端口，供外地办公人员访问内网。

## 在内网服务器安装 natapp

从 https://natapp.cn/#download 获得下载链接，使用 wget 进行下载，同时修改执行权限

```bash
$ wget https://cdn.natapp.cn/assets/downloads/clients/2_4_0/natapp_linux_amd64/natapp
$ chmod a+x natapp

$ mv natapp /usr/local/natapp/natapp
```

## 购买 natapp 隧道

> https://natapp.cn/article/natapp_newbie
>
> https://natapp.cn/article/tcp

1. 注册帐号 -> 购买隧道 -> VIP-1型 -> [填写如下信息]
2. 隧道协议: TCP
3. 远程端口: 任意一个没有使用的端口
4. 本地端口: 如果是SSH，则填写 22

## 配置内网服务器的开机自启动服务

将 [natapp.service](https://github.com/natapp/natapp_autostart/tree/master)
保存到 `/usr/lib/systemd/system/natapp.service`，记得将 authtoken 修改成
`我的隧道 -> authtoken`

```bash
$ systemctl enable natapp
$ systemctl start natapp
```

## ssh 远程访问内网服务器

从`我的隧道 -> 域名/端口` 获得 `xxx.natapp.cc` and `remote_port`

```bash
$ ssh local_username@xxx.natapp.cc -p remote_port
```
