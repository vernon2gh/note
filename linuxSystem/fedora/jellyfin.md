# 简介

The Free Software Media System.

Jellyfin is the volunteer-built media solution that puts you in control of your
media. Stream to any device from your own server, with no strings attached.
Your media, your server, your way.

# Server

> https://jellyfin.org/docs/general/installation/
> https://jellyfin.org/docs/general/installation/container#podman

参考以上官方文档，主要操作是安装 podman，通过 podman 启动 jellyfin 容器，
并且添加 firewall 允许端口。

最后将相关电影存储到 `/path/to/media` 目录中，这样在 `http://IP:8096` 配置界面中
通过 `/media` 目录就能够访问所有相关电影。

# Clients

> https://jellyfin.org/downloads

第一方式，通过浏览器直接访问 `http://IP:8096` 即可。
第二方式，下载对应的 APP，如：Jellyfin for Android TV
