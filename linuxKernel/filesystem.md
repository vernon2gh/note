# 简介

```
user sapce          read()/write() syscall
                ----------------------------
kernel space        vfs
                    ext4/xfs/bcachefs
                    BIO
                    IO scheduler
                    block driver
                ----------------------------
hardware            disk
```

我们平时所说的 superblock 对象，inode 对象，dentry 对象，file 对象 分别代表
`struct super_block`、`struct inode`、`struct dentry`、`struct file` 结构体。

* 每打开一个文件，创建一个 file 对象。
* 每创建一个目录，创建一个 dentry 对象。
* 每创建一个新文件，创建一个 inode 对象。
* 每格式化一个新文件系统，创建一个 superblock 对象。

实现一个文件系统，实际上就是实现 `super_operations`, `inode_operations`,
`dentry_operations` 结构体（`file_operations` 等于 `inode_operations`），
这样 vfs 才能正常地使用文件系统进行工作。

# open() 系统调用返回的 FD

```
struct task_struct {
	struct files_struct		*files;
}

struct files_struct {
	struct fdtable __rcu *fdt;
}

struct fdtable {
	struct file __rcu **fd;
}
```

用户空间使用 `fd = open(filename)` 打开一个文件后，返回一个整型 fd。
这个整型 fd 就是 `task.files.fdtable.fd[]` 数组的下标。

每一次调用 `open()` 都会分配一个新 `struct file`，保存在 `task.files.fdtable.fd[]`

# 如何初始化 `file->inode` 成员

`open(filename)` 通过 filename 找到 dentry 对象，dentry对象能够找到 inode 对象，
最后将 inode 对象赋值给 `file->inode` 成员

# 如何初始化 fd 结构体

```
struct fd {
	struct file *file;
	unsigned int flags;
};
```

在使用 `write(fd)` 系统调用时，传递整型 fd 下来，初始化 `fd.file = task.files.fdtable.fd[fd]`

