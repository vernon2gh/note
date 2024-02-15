```bash
$ lsblk
NAME                MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
zram0               252:0    0     8G  0 disk [SWAP]
nvme0n1             259:0    0 476.9G  0 disk
├─nvme0n1p1         259:1    0   600M  0 part /boot/efi
├─nvme0n1p2         259:2    0     1G  0 part /boot
└─nvme0n1p3         259:3    0 475.4G  0 part
  └─fedora_192-root 253:0    0    15G  0 lvm  /
```

默认安装 fedora 系统后，`/` 目录只有 15GB 存储空间。

```bash
$ sudo lvextend -l +100%FREE --resizefs /dev/mapper/fedora_192-root
  Size of logical volume fedora_192/root changed from 15.00 GiB (3840 extents) to 475.35 GiB (121690 extents).
meta-data=/dev/mapper/fedora_192-root isize=512    agcount=4, agsize=983040 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=1, sparse=1, rmapbt=0
         =                       reflink=1    bigtime=1 inobtcount=1 nrext64=0
data     =                       bsize=4096   blocks=3932160, imaxpct=25
         =                       sunit=0      swidth=0 blks
naming   =version 2              bsize=4096   ascii-ci=0, ftype=1
log      =internal log           bsize=4096   blocks=16384, version=2
         =                       sectsz=512   sunit=0 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
data blocks changed from 3932160 to 124610560
  Logical volume fedora_192/root successfully resized.
```

使用 lvextend 命令将 `/` 目录的存储空间设置成最大空闲存储空间值。

```bash
$ lsblk
NAME                MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
zram0               252:0    0     8G  0 disk [SWAP]
nvme0n1             259:0    0 476.9G  0 disk
├─nvme0n1p1         259:1    0   600M  0 part /boot/efi
├─nvme0n1p2         259:2    0     1G  0 part /boot
└─nvme0n1p3         259:3    0 475.4G  0 part
  └─fedora_192-root 253:0    0 475.4G  0 lvm  /
```

使用 lvextend 调整后，`/` 目录有 475.4GB 存储空间。
