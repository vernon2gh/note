# 升级系统版本

更新系统版本的信息，这是必需的。因为升级系统需要高版本的签名密钥，它们通常会
修复与升级过程相关的问题。

```bash
$ sudo dnf clean all        ## option
$ sudo dnf upgrade --refresh
$ sudo reboot
```

下载高版本的升级包

```bash
$ sudo dnf system-upgrade download --releasever=43
```

开始升级系统版本，并且重启。

```bash
$ sudo dnf offline reboot
```

等待升级完成即可。

# 升级后的可选操作

清理旧内核

```bash
#!/usr/bin/env bash

old_kernels=($(dnf repoquery --installonly --latest-limit=-1 -q))
if [ "${#old_kernels[@]}" -eq 0 ]; then
    echo "No old kernels found"
    exit 0
fi

if ! dnf remove "${old_kernels[@]}"; then
    echo "Failed to remove old kernels"
    exit 1
fi

echo "Removed old kernels"
exit 0
```

随着每个版本的发布，Fedora 都会淘汰一些软件包。原因有多种；软件包变得过时，
它们的上游不维护。它们仍然在您的系统上，但是这些软件包将不会获得升级。
强烈建议删除这些软件包。

```bash
$ sudo dnf install remove-retired-packages
$ remove-retired-packages
```

# 参考

https://docs.fedoraproject.org/zh_CN/quick-docs/upgrading-fedora-offline/
