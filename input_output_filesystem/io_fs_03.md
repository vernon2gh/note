# 文件系统的实现

### EXT2/3/4的layout

1. Super block, stoing:

   size and location of bitmaps

   number and location of inodes

   number and location of data blocks

   index of root inodes

2. Bitmap of free & used indoes

3. Bitmap of free & used data blocks

4. Table of inodes

   each indo is a file/directory

   includes meta-data and lists of associated data blocks

5. Data blocks(4kB each)

EXT2/3/4是以group进行分组的，

superblock记录文件系统的类型、block大小、block总数、free block数、inode大小、inode总数、free inode数、group的总数等，在多个group进行备份

group描述符记录：block bitmap位置、inode bitmap位置、inode表位置、free block、free inode数量

### 文件系统的一致性

对一个文件增加4k大小，需要对indoes Bitmap、data blocks Bitmap 、Table of inodes、Data blocks进行修改。如果只修改了其中几项，突然断电了，文件系统就不一致。

例子：

```bash
## 创建一个4M的镜像文件
$ dd if=/dev/zero of=Image bs=1024 count=4096
## 将镜像文件格式化成ext4格式
$ mkfs.ext4 -b 4096 Image
## 查看inodes bitmap位于第x块
$ dumpe2fs Image 
Inode 位图位于 18 (+18)
## 查看镜像文件对应的inodes bitmap数据
$ dd if=Image bs=4096 skip=18 | hexdump -C -n 32
00000000  ff 07 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020

## inodes bitmap数据为[ff 07]，即前11个inode都被使用了
## 如果在此文件系统创建新文件，分配的inode number就是12，如下
$ sudo mount -o loop Image test/
$ cd test/
$ sudo touch yes
$ ls -li yes
12 -rw-r--r-- 1 root root 0 11月 13 19:13 yes
$ dd if=Image bs=4096 skip=18 | hexdump -C -n 32
00000000  ff 0f 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020

## 下面制造一个"断电"事故，导致inodes bitmap没有正常修改
## 将ff 0f修改为ff 07
$ hexedit Image
$ dd if=Image bs=4096 skip=18 | hexdump -C -n 32
00000000  ff 07 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020
$ sudo mount -o loop Image test/
$ cd test/
$ ls -li
11 drwx------ 2 root root 16384 11月 13 19:07 lost+found
12 -rw-r--r-- 1 root root     0 11月 13 19:13 yes
## 因为此文件系统不一致性，导致fail
$ sudo touch no
touch: 无法创建 'no': 设备上没有空间
$ dmesg | tail
[  604.710098] EXT4-fs (loop1): mounted filesystem without journal. Opts: (null)
[  604.710113] ext4 filesystem being mounted at /home/vernon/workplaces/test/io-courses/test supports timestamps until 2038 (0x7fffffff)
[  907.625269] EXT4-fs (loop1): mounted filesystem without journal. Opts: (null)
[  907.625287] ext4 filesystem being mounted at /home/vernon/workplaces/test/io-courses/test supports timestamps until 2038 (0x7fffffff)
[ 1568.654614] EXT4-fs (loop1): mounted filesystem without journal. Opts: (null)
[ 1568.654630] ext4 filesystem being mounted at /home/vernon/workplaces/test/io-courses/test supports timestamps until 2038 (0x7fffffff)
[ 1582.104416] EXT4-fs error (device loop1): ext4_validate_inode_bitmap:99: comm touch: Corrupt inode bitmap - block_group = 0, inode_bitmap = 18
$ ls -li
11 drwx------ 2 root root 16384 11月 13 19:07 lost+found
12 -rw-r--r-- 1 root root     0 11月 13 19:13 yes
```

### 掉电与文件系统一致性

1. 任何的软件技术都**无法保证掉电不丟数据，只能保证一致性**（元数据+数据的一致性 或者 仅元数据的一致性）
2. dirty_expire_centisecs、DIRECT_IO、SYNC IO的调整，不影响丟/不丟数据，只影响丟多少数据
3. **fsck**、**日志**、**COW文件系统**等技术，帮忙提供**一致性**

### fsck

针对早期文件系统，系统重新启动中，使用fsck提供一致性，修复前面的"断电"事故

fsck, file system consistency check

unclean shotdown后自动运行，或者手动运行; 检查superblock、inode和free block bitmap、所有inode的reachability（比如删除corrupted的inode）、验证目录的一致性

例子：

```bash
$ dd if=Image bs=4096 skip=18 | hexdump -C -n 32
00000000  ff 07 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020
$ fsck.ext4 -f Image
e2fsck 1.45.5 (07-Jan-2020)
第 1 步：检查inode、块和大小
第 2 步：检查目录结构
第 3 步：检查目录连接性
第 4 步：检查引用计数
第 5 步：检查组概要信息
Inode位图的差异：  +12
处理<y>? 是

Image：***** 文件系统已修改 *****
Image：12/1024 文件（0.0% 为非连续的）， 42/1024 块
$ dd if=Image bs=4096 skip=18 | hexdump -C -n 32
00000000  ff 0f 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020
```

### 文件系统的日志

针对ext2/ext3/ext4文件系统，对文件系统的修改进行日志管理，提供一致性，从而修复突然断电事故

保存元数据+数据日志(data=journal) - 4个阶段：

1. jonrnal write
2. journal commit
3. jonrnal checkpoint
4. jonrnal free

只保存元数据日志(data=writeback or data=ordered) - 5个阶段：

1. data write
2. journal metadata write
3. journal commit
4. jonrnal checkpoint metadata
5. jonrnal free

### Copy On Write文件系统

针对btrfs文件系统

每次写磁盘时，先将更新数据写入一个新的block，当新数据写入成功之后，再更新相关的数据结构指向新block

没有日志，用COW实现文件系统一致性

### 文件系统的debug和dump

常用工具：mkfs、dumpe2f、blkcat、dd、debugfs、blktrace

```bash
## block_number = 11591720
## inode_number = 2097400
$ debugfs -R 'stat /home/vernon/test.txt' /dev/sda1
Inode: 2097400   Type: regular    Mode:  0644   Flags: 0x80000
Generation: 477007534    Version: 0x00000000:00000001
User:  1000   Group:  1000   Size: 14
File ACL: 0    Directory ACL: 0
Links: 1   Blockcount: 8
Fragment:  Address: 0    Number: 0    Size: 0
 ctime: 0x5fae92a4:360c2a44 -- Fri Nov 13 22:05:24 2020
 atime: 0x5fae92a4:360c2a44 -- Fri Nov 13 22:05:24 2020
 mtime: 0x5fae92a4:360c2a44 -- Fri Nov 13 22:05:24 2020
crtime: 0x5fae92a4:360c2a44 -- Fri Nov 13 22:05:24 2020
Size of extra inode fields: 28
EXTENTS:
(0): 11591720

## 通过/dev/sda1打印/home/vernon/test.txt内容
$ blkcat /dev/sda1 11591720
or
$ dd if=/dev/sda1 of=out.txt skip=$((11591720*8)) bs=512c count=1

## /dev/sda1 offset = 2048
$ fdisk /dev/sda
## 通过/dev/sda打印/home/vernon/test.txt内容
$ dd if=/dev/sda of=out.txt skip=$((11591720*8+2048)) bs=512c count=1

## block number look for inode number
$ debugfs -R 'icheck 11591720' /dev/sda1
debugfs 1.41.11 (14-Mar-2010)
Block	Inode number
11591720	2097400

## inode number look for file name
$ debugfs -R 'ncheck 2097400' /dev/sda1
debugfs 1.41.11 (14-Mar-2010)
Inode	Pathname
2097400	/home/vernon/test.txt
```

