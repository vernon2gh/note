ubuntu10.04LTS官方已经不再维护，导致部分软件、库文件等下载有问题，可换以下源地址解决问题

```bash
$ cat sources.list
deb http://old-releases.ubuntu.com/ubuntu lucid main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu lucid-security main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu lucid-updates main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu lucid-proposed main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu lucid-backports main restricted universe multiverse 
deb-src http://old-releases.ubuntu.com/ubuntu lucid main restricted universe multiverse
deb-src http://old-releases.ubuntu.com/ubuntu lucid-security main restricted universe multiverse
deb-src http://old-releases.ubuntu.com/ubuntu lucid-updates main restricted universe multiverse
deb-src http://old-releases.ubuntu.com/ubuntu lucid-proposed main restricted universe multiverse
deb-src http://old-releases.ubuntu.com/ubuntu lucid-backports main restricted universe multiverse
```