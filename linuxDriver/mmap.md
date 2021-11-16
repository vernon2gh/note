### 简介

首先在驱动程序分配一页大小的内存，然后用户进程通过`mmap()`将用户空间中大小也为一页的内存映射到内核空间这页内存上。映射完成后，驱动程序往这段内存写10个字节数据，用户进程将这些数据显示出来

### 应用空间操作

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PAGE_SIZE 4096

int main(int argc , char *argv[])
{
    int fd;
    int i;
    unsigned char *p_map;

    fd = open("/dev/mymap",O_RDWR);
    p_map = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0);

    for(i=0;i<10;i++)
        printf("%d\n",p_map[i]);

    munmap(p_map, PAGE_SIZE);
    return 0;
}
```

### 内核空间操作

```c
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>

#define DEVICE_NAME "mymap"

static unsigned char array[10]={0,1,2,3,4,5,6,7,8,9};
static unsigned char *buffer;

static int my_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int my_map(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long page;
    unsigned char i;
    unsigned long start = (unsigned long)vma->vm_start;
    unsigned long end =  (unsigned long)vma->vm_end;
    unsigned long size = (unsigned long)(end - start);

    page = virt_to_phys(buffer); // 得到物理地址

    // 将用户空间的一个vma虚拟内存区映射到以page开始的一段连续物理页面上
    if(remap_pfn_range(vma, start, page>>PAGE_SHIFT, size, PAGE_SHARED))
        return -1;

    for(i=0;i<10;i++)
        buffer[i] = array[i];

    return 0;
}

static struct file_operations dev_fops = {
    .owner  = THIS_MODULE,
    .open   = my_open,
    .mmap   = my_map,
};

static struct miscdevice misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &dev_fops,
};

static int __init dev_init(void)
{
    int ret;

    ret = misc_register(&misc);
    buffer = (unsigned char *)kmalloc(PAGE_SIZE,GFP_KERNEL);
    SetPageReserved(virt_to_page(buffer)); // 将该段内存设置为保留

    return ret;
}

static void __exit dev_exit(void)
{
    misc_deregister(&misc);
    ClearPageReserved(virt_to_page(buffer)); // 清除保留
    kfree(buffer);
}

module_init(dev_init);
module_exit(dev_exit);
```

### 参考

[示例1-驱动+应用](https://nieyong.github.io/wiki_cpu/mmap%E8%AF%A6%E8%A7%A3.html)

