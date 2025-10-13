## 简介

Apache HTTP 服务器是一个开源的、跨平台的 Web 服务器软件，由 Apache 软件基金会
开发和维护，广泛用于托管网站和应用程序。

在 Fedora 中使用 httpd 作为包名，所以通过 `dnf install httpd` 命令来安装。
在 Debian 中使用 apache2 作为包名，所以通过 `apt install apache2` 命令来安装。

Fedora‌ 的配置文件通常位于 `/etc/httpd` 目录下。
Debian 的配置文件通常位于 `/etc/apache2` 目录下。

默认网站根目录都是 `/var/www/html`

下面以 Fedora 为例子

## 下载

```bash
$ dnf install httpd
```

## 配置环境

以搭建 `mmtests-mirror` Apache HTTP 服务器为例

```bash
$ echo "127.0.0.1 mcp" >> /etc/hosts              ## 可选
$ mkdir /var/www/mmtests-mirror
$ cat /etc/httpd/conf.d/mmtests-mirror.conf
Alias /mmtests-mirror/ "/var/www/mmtests-mirror/" ## 将 /var/www/mmtests-mirror 映射到 http://mcp/mmtests-mirror
<Directory "/var/www/mmtests-mirror">
    Options Indexes FollowSymLinks
    AllowOverride None
    Require all granted
</Directory>

$ systemctl enable httpd
$ systemctl start httpd
```

## 使用

```bash
$ cd /var/www/mmtests-mirror
$ ls
kernel/linux-6.15.tar.xz

$ wget http://mcp/mmtests-mirror/kernel/linux-6.15.tar.xz
```

下载 /var/www/mmtests-mirror 的 kernel/linux-6.15.tar.xz 文件
