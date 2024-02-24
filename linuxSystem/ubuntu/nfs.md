## NFS 服务器

* 安装

```bash
$ sudo apt install nfs-kernel-server
```

* 配置

```bash
## 添加共享目录
$ vim /etc/exports
<shareFileDir> *(rw,sync,no_root_squash)

## 测试
$ showmount -e
```

## NFS 客户端

* linux 使用 NFS

```bash
$ mount -t nfs <server IP>:<shareFileDir> <mountDir>
```

* windows 使用 NFS（安装NFS服务）

```powershell
\\<server IP>
```
