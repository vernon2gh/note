### 简介

系统调用(system call)是用户空间(user space)与内核空间(kernel space)通信的桥梁，其中应用层 glibc/uclibc库 等的实现，就是对系统调用进行封装，所以想从应用层向内核层深入研究，必须先对系统调用有一定了解。

### 零参数系统调用

1. 内核层添加系统调用

* 系统调用入口

  ```bash
  $ vim arch/x86/include/asm/unistd_64.h
  #define __NR_foo				300
  __SYSCALL(__NR_foo, sys_foo)
  ```

* 系统调用实现

  ```c
  $ vim kernel/sys.c
  #include <linux/syscalls.h>
  
  SYSCALL_DEFINE0(foo)
  {
  	printk("%s: syscall test\n", __func__);
  	return 0;
  }
  ```

2. 应用层调用系统调用

   ```c
   $ vim syscall.c
   #include <stdio.h>
   #include <unistd.h>
   #include <sys/syscall.h>
   
   #define SYSCALL_FOO		300 
   
   void syscall_0(void)
   {
   	syscall(SYSCALL_FOO);
   }
   ```
