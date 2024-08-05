通过 `/etc/os-release` 查看操作系统的版本，如下：

```bash
## ubuntu
$ cat /etc/os-release
PRETTY_NAME="Ubuntu 22.04.4 LTS"
NAME="Ubuntu"
VERSION_ID="22.04"
VERSION="22.04.4 LTS (Jammy Jellyfish)"
VERSION_CODENAME=jammy
ID=ubuntu
ID_LIKE=debian
...
UBUNTU_CODENAME=jammy

## fedora
$ cat /etc/os-release
NAME="Fedora Linux"
VERSION="38 (Server Edition)"
ID=fedora
VERSION_ID=38
VERSION_CODENAME=""
PLATFORM_ID="platform:f38"
PRETTY_NAME="Fedora Linux 38 (Server Edition)"
...
VARIANT="Server Edition"
VARIANT_ID=server

```
