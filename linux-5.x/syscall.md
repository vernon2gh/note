### 简介

系统调用(system call)是用户空间(user space)与内核空间(kernel space)通信的桥梁，其中应用层 glibc/uclibc库 等的实现，就是对系统调用进行封装，所以想从应用层向内核层深入研究，必须先对系统调用有一定了解。

### 零参数系统调用

1. 内核层添加系统调用

* 系统调用入口

  ```bash
  $ vim arch/x86/entry/syscalls/syscall_64.tbl
  436	common	syscall_0			__x64_sys_syscall_0
  ```

* 系统调用实现

  ```c
  $ vim fs/syscall.c
  #include <linux/syscalls.h>
  
  SYSCALL_DEFINE0(syscall_0)
  {
  	printk("%s\n", __func__);
  	return 0;
  }
  ```

2. 应用层调用系统调用

   ```c
   $ vim syscall.c
   #include <stdio.h>
   #include <unistd.h>
   #include <sys/syscall.h>
   
   #define SYSCALL_0		436 
   
   void syscall_0(void)
   {
   	syscall(SYSCALL_0);
   }
   ```

### 整型参数系统调用

1. 内核层添加系统调用

* 系统调用入口

  ```bash
  $ vim arch/x86/entry/syscalls/syscall_64.tbl
  437	common	syscall_int1		__x64_sys_syscall_int1
  438	common	syscall_int2		__x64_sys_syscall_int2
  ```

* 系统调用实现

  ```c
  $ vim fs/syscall.c
  #include <linux/syscalls.h>
  
  SYSCALL_DEFINE1(syscall_int1, int, parameter1)
  {
  	printk("%s: %d\n", __func__, parameter1);
  	return 0;
  }
  
  SYSCALL_DEFINE2(syscall_int2, int, parameter1, int, parameter2)
  {
  	printk("%s: %d, %d\n", __func__, parameter1, parameter2);
  	return 0;
  }
  ```

2. 应用层调用系统调用

   ```c
   $ vim syscall.c
   #include <stdio.h>
   #include <unistd.h>
   #include <sys/syscall.h>
   
   #define SYSCALL_INT1	437
   #define SYSCALL_INT2	438
   
   void syscall_int1(void)
   {
   	int p1 = 0x01;
   
   	syscall(SYSCALL_INT1, p1);
   }
   
   void syscall_int2(void)
   {
   	int p1 = 0x01;
   	int p2 = 0x02;
   
   	syscall(SYSCALL_INT2, p1, p2);
   }
   ```

### 字符串参数系统调用

1. 内核层添加系统调用

* 系统调用入口

  ```bash
  $ vim arch/x86/entry/syscalls/syscall_64.tbl
  439	common	syscall_str1		__x64_sys_syscall_str1
  ```

* 系统调用实现

  ```c
  $ vim fs/syscall.c
  #include <linux/syscalls.h>
  #include <linux/string.h>
  
  SYSCALL_DEFINE1(syscall_str1, char __user *, parameter1)
  {
  	char kbuf[64];
  	unsigned long ret;
  
  	ret = copy_from_user(kbuf, parameter1, strlen(parameter1)+1);
  	printk("%s: user to kernel: %s\n", __func__, kbuf);
  
  	strcpy(kbuf, "kernel");
  	ret = copy_to_user(parameter1, kbuf, strlen(kbuf)+1);
  
  	return 0;
  }
  ```

2. 应用层调用系统调用

   ```c
   $ vim syscall.c
   #include <stdio.h>
   #include <unistd.h>
   #include <sys/syscall.h>
   
   #define SYSCALL_STR1	439
   
   void syscall_str1(void)
   {
   	char p1[64] = "hello world";
   
   	syscall(SYSCALL_STR1, p1);
   	printf("kernel to user: %s\n", p1);
   }
   ```

### 数组参数系统调用

1. 内核层添加系统调用

* 系统调用入口

  ```bash
  $ vim arch/x86/entry/syscalls/syscall_64.tbl
  440	common	syscall_array1		__x64_sys_syscall_array1
  ```

* 系统调用实现

  ```c
  $ vim fs/syscall.c
  #include <linux/syscalls.h>
  
  SYSCALL_DEFINE1(syscall_array1, int __user *, parameter1)
  {
  	int karray[3];
  	unsigned long ret;
  
  	ret = copy_from_user(karray, parameter1, sizeof(int) * 3);
  	printk("%s: %d %d %d\n",
  		__func__, karray[0], karray[1], karray[2]);
  
  	return 0;
  }
  ```

2. 应用层调用系统调用

   ```c
   $ vim syscall.c
   #include <stdio.h>
   #include <unistd.h>
   #include <sys/syscall.h>
   
   #define SYSCALL_ARRAY1	440
   
   void syscall_array1(void)
   {
   	int p1[3] = {3, 4, 5};
   
   	syscall(SYSCALL_ARRAY1, p1);
   }
   ```

### 指针参数系统调用

1. 内核层添加系统调用

* 系统调用入口

  ```bash
  $ vim arch/x86/entry/syscalls/syscall_64.tbl
  441	common	syscall_pointer1	__x64_sys_syscall_pointer1
  ```

* 系统调用实现

  ```c
  $ vim fs/syscall.c
  #include <linux/syscalls.h>
  
  SYSCALL_DEFINE1(syscall_pointer1, int __user *, parameter1)
  {
  	int ktest;
  	unsigned long ret;
  
  	ret = copy_from_user(&ktest, parameter1, sizeof(int));
  	printk("%s: %d\n", __func__, ktest);
  
  	return 0;
  }
  ```

2. 应用层调用系统调用

   ```c
   $ vim syscall.c
   #include <stdio.h>
   #include <unistd.h>
   #include <sys/syscall.h>
   
   #define SYSCALL_POINTER1	441
   
   void syscall_pointer1(void)
   {
   	int p1 = 6;
   
   	syscall(SYSCALL_POINTER1, &p1);
   }
   ```

