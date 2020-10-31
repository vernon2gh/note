> 此文档属于解决类文档，直接解决遇到的问题。

### 1. VNC server running on `127.0.0.1:5900'

```bash
$ sudo apt search libsdl
libsdl2-dev/bionic-updates,bionic-security,now 2.0.8+dfsg1-1ubuntu1.18.04.4 amd64
  Simple DirectMedia Layer development files
$ sudo apt install libsdl2-dev
# 重新编译qemu即可
```

