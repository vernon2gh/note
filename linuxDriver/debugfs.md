## 0. 简述

linux kernel debugfs API，可以帮助linux驱动工程师快速调试某个外设驱动，在应用层通过cat/echo命令修改内核层数据，目录位于`/sys/kernel/debug/`

### 1. linux内核层

#### 1.1 头文件

```c
#include <linux/debugfs.h>
```

#### 1.2 创建目录、文件

```c
struct dentry *dir;

const struct file_operations fops = {
    .read    = debugfs_read,
    .write   = debugfs_write,
};

/* 
 * "dir"：目录名字
 * NULL ：在/sys/kernel/debug创建目录
 * 即 /sys/kernel/debug/dir/
 */
dir = debugfs_create_dir("dir", NULL);

/* 
 * "data"：文件名字
 * 0644  ：文件权限
 * dir   ：父目录
 * &fops：文件对应的操作函数
 * 即 /sys/kernel/debug/dir/data
 */
debugfs_create_file("data", 0644, dir, NULL, &fops); 
```

#### 1.3 删除目录、文件

```c
/*
 * 删除/sys/kernel/debug/dir目录，包括子目录
 */
debugfs_remove_recursive(dir);
```

#### 1.4 编写read/write函数

```c
unsigned char data = 'd';

ssize_t debugfs_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    unsigned char kbuf[3];

    kbuf[0] = data;
    kbuf[1] = '\n'; // 可选
    kbuf[2] = '\0'; // 可选

    return simple_read_from_buffer(buf, size, offset, kbuf, sizeof(kbuf));
}

ssize_t debugfs_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    unsigned char *kbuf;

    kbuf = kmalloc(size, GFP_KERNEL);

    copy_from_user(kbuf, buf, size);
    kbuf[size-1] = '\0'; // 可选
    data = kbuf[0];

    kfree(kbuf);

    return size;
}
```

**注意：**在debugfs_read()函数，不能用copy_to_user()函数，必须用simple_read_from_buffer()函数，具体原因，请看[《使用cat读取和echo写内核文件节点的一些问题》](https://www.cnblogs.com/pengdonglin137/p/8012793.html)

### 2. linux应用层

```bash
$ cd /sys/kernel/debug
$ echo y > dir/data
$ cat dir/data
y
```

