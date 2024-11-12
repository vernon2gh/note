## 打包参数

```
rpm-pkg             - Build both source and binary RPM kernel packages
srcrpm-pkg          - Build only the source kernel RPM package            ## kernel 源码包
binrpm-pkg          - Build only the binary kernel RPM package            ## vmlinux/modules 固件
deb-pkg             - Build both source and binary deb kernel packages
srcdeb-pkg          - Build only the source kernel deb package            ## kernel 源码包
bindeb-pkg          - Build only the binary kernel deb package            ## vmlinux/modules 固件包
snap-pkg            - Build only the binary kernel snap package
                        (will connect to external hosts)
pacman-pkg          - Build only the binary kernel pacman package
dir-pkg             - Build the kernel as a plain directory structure     ## vmlinux/modules 固件包
tar-pkg             - Build the kernel as an uncompressed tarball
targz-pkg           - Build the kernel as a gzip compressed tarball
tarbz2-pkg          - Build the kernel as a bzip2 compressed tarball
tarxz-pkg           - Build the kernel as a xz compressed tarball
tarzst-pkg          - Build the kernel as a zstd compressed tarball
perf-tar-src-pkg    - Build the perf source tarball with no compression   ## perf 源码包
perf-targz-src-pkg  - Build the perf source tarball with gzip compression
perf-tarbz2-src-pkg - Build the perf source tarball with bz2 compression
perf-tarxz-src-pkg  - Build the perf source tarball with xz compression
perf-tarzst-src-pkg - Build the perf source tarball with zst compression
```
