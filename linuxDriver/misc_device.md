## 简述

linux kernel miscdevice编写，需要三步：

* 创建 struct file_operations 变量
* 创建 struct miscdevice 变量
* 注册/注销miscdevice

#### 1. 创建 struct file_operations 变量

```c
const struct file_operations xxx_fops = {
	.owner    = THIS_MODULE,
	.open     = xxx_open,
	.release  = xxx_release,
	.write    = xxx_write,
	.read     = xxx_read,
};
```

#### 2. 创建 struct miscdevice 变量

```c
struct miscdevice xxx_miscdevice = {
	.minor = SUB_DEVICE_NUMBER, // 子设备号，只要linux miscdevice没有使用的子设备号都可以
	.name  = "xxxDev",          // 设备文件名/dev/xxxDev
	.fops  = &xxx_fops,
};
```

#### 3. 注册/注销miscdevice

```c
misc_register(&xxx_miscdevice);  // 注册
misc_deregister(&xxx_miscdevice);// 注销
```

