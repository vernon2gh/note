## TTFP 服务器

* 安装

```bash
$ sudo apt install tftpd-hpa
```

* 配置

```bash
$ sudo nvim /etc/default/tftpd-hpa

TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/srv/tftp"  ## 默认 TFTP 目录
TFTP_ADDRESS=":69"
TFTP_OPTIONS="--secure -c"  ## 默认只允许下载文件，添加 -c 参数代表允许上传文件

## 当设置允许上传文件时，需要将 TFTP 目录设置成所有人可写
$ sudo chmod a+w /srv/tftp

## 修改 TFTP 配置文件后，需要重新启动 TFTP 服务器
$ sudo systemctl restart tftpd-hpa.service
```

## TFTP 客户端

* 安装

```bash
$ sudo apt install tftp-hpa
```

* 使用

```bash
$ tftp [server IP]
tftp> get [download_file]
tftp> put [upload_file]
```
