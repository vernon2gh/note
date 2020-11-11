## 文件系统的架构

### 一切都是文件：VFS

应用层的`read/write/ioctl/open`都是VFS实现的虚函数，所以内核层需要提供`struct file_operations`，这样这此虚函数才可以生效。

例子1：各种字符设备驱动 需要实现`struct file_operations`

```c
$ vim drivers/char/misc.c
static const struct file_operations misc_proc_fops = {
        .read    = seq_read,
        ...
};
```

例子2：Block RAW设备 需要实现`struct file_operations`

```c
$ dd if=/dev/sda1 ...
$ cat /dev/sda1

$ vim fs/block_dev.c
const struct file_operations def_blk_fops = {
        .read           = new_sync_read,
        .write          = new_sync_write,
        ...
};
```

例子3：Ext4中的file 需要实现`struct file_operations`

```c
$ mount /dev/sda1 /mnt
$ ls /mnt
1.txt 2.txt test.out

$ vim fs/ext4/file.c
const struct file_operations ext4_file_operations = {
        .read           = new_sync_read,
        .write          = new_sync_write,
        ...
};
```

### 文件系统的管理：超级块、目录、inode

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

##### Inode diagram

Inode diagram需要描述文件存放在磁盘的哪些块(block)

##### Indirect blocks

Inode diagram不足以描述足够多的磁盘块，因此可以透过间接 指向的方法，支持大文件

##### EXT4 Extents

一个Extents是一个地址连续的数据块(block)的集合。比如一个100MB的 文件有可能被分配给一个单独的Extents，这样就不用像Ext3那样新增 25600个数据块的记录（假设一个数据块是4KB）

##### xxx_inode_cache

更低层次的cache，描述了inode diagram里面的东西，与硬盘联系更加紧密

##### Directory diagram

目录是一个特殊的文件，是inode号与名字的映射表

### 硬链接与软链接

* 硬链接，inodes号是一样

```bash
##　注意：创建a.txt 1分钟后，才创建硬链接b.txt
$ ln a.txt b.txt
$ ls -il *.txt
50725039 -rw-rw-r-- 2 vernon vernon 0 11月 10 20:21 a.txt
50725039 -rw-rw-r-- 2 vernon vernon 0 11月 10 20:21 b.txt
```

b.txt是a.txt的硬链接，所以在Table of inodes中，a.txt inodes number = b.txt inodes number

* 软链接，也叫符号链接，inodes号是不同的

```bash
##　注意：创建a.txt 1分钟后，才创建软链接c.txt
$ ln -s a.txt c.txt
$ ls -il *.txt
50725039 -rw-rw-r-- 2 vernon vernon 0 11月 10 20:21 a.txt
50725040 lrwxrwxrwx 1 vernon vernon 5 11月 10 20:22 c.txt -> a.txt
```

b.txt是a.txt的软链接，所以在Table of inodes中，a.txt inodes number != b.txt inodes number

### icache和dcache

icache, inode cache

dcache, dentry cache

### 用户空间的文件系统:FUSE

FUSE需要把VFS层的请求传到用户态的fuse app，在用户态处理，然 后再返回到内核态，把结果返回给VFS层；在用户态实现文件系统必 然会引入额外的内核态/用户态切换带来的开销，对性能会产生一定影响
