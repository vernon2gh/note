## 简述

KVM (Kernel-based Virtual Machine)，全称：基于内核的虚拟机，以内核模块的形式存在。

如果 Linux 系统支持 KVM，即存在设备节点 `/dev/kvm`，直接使用 `ioctl()` 进行操作。
主要步骤如下：

```c
open("/dev/kvm")                 // 打开设备节点
ioctl(KVM_GET_API_VERSION)       // 获得KVM API版本（可选）

ioctl(KVM_CREATE_VM)             // 创建虚拟机
ioctl(KVM_SET_USER_MEMORY_REGION)// 设置虚拟机内存

ioctl(KVM_CREATE_VCPU)           // 创建虚拟 CPU
ioctl(KVM_GET_VCPU_MMAP_SIZE)    // 获得 kvm_run 结构体大小

ioctl(KVM_GET_SREGS)             // 获得特权寄存器值
ioctl(KVM_SET_SREGS)             // 设置特权寄存器值

ioctl(KVM_GET_REGS)              // 获得普通寄存器值
ioctl(KVM_SET_REGS)              // 设置普通寄存器值

ioctl(KVM_RUN)                   // 运行虚拟机
```

## kvm 操作了解

主要阅读 [Using the KVM API](https://lwn.net/Articles/658511/) 文章，即可知道原理。

根据以上文章编写的测试源码 [kvmtest.c](https://github.com/vernon2gh/note/tree/main/resources/code/c/kvm/kvmtest.c)

## kvmtool 使用

```bash
$ git clone git://git.kernel.org/pub/scm/linux/kernel/git/will/kvmtool.git
$ cd kvmtool && make
$ wget http://lassauge.free.fr/qemu/release/linux-0.2.img.bz2 && bunzip2 linux-0.2.img.bz2
$ ./lkvm run --disk linux-0.2.img --kernel arch/x86/boot/bzImage
```

具体详细阅读 kvmtool README

## kvmtool的详细实现

以 ./lkvm run 子命令为例：

```c
main()
    handle_kvm_command()
        handle_command()
            kvm_cmd_run()
```

根据命令参数 `run` 依次在静态数组 `kvm_commands[]` 中进行对比，获得 run 子命令的执行函数，
即为 kvm_cmd_run()

```c
kvm_cmd_run()
    kvm_cmd_run_init()
        init_list__init()
            kvm__init()
            kvm_cpu__init()
    kvm_cmd_run_work()
        kvm_cpu_thread()
            kvm_cpu__start()
                kvm_cpu__reset_vcpu()
                kvm_cpu__run()
```

kvmtool 在 main() 函数执行之前，会执行一些初始化函数，如 core_init()、base_init() 等等，
这些 xxx_init(func) 会将 func 放在哈希链表里面。

当调用 init_list__init() 时，依次执行哈希链表的 func()，进行一系列的初始化，
比如： kvm__init() `KVM_CREATE_VM KVM_SET_USER_MEMORY_REGION`、
kvm_cpu__init() `KVM_CREATE_VCPU KVM_GET_VCPU_MMAP_SIZE`

最后为每一个虚拟 CPU 生成一个 thread，thread 执行函数为 kvm_cpu_thread()，主要是
复位虚拟 CPU `KVM_SET_SREGS KVM_SET_REGS`、运行虚拟机 `KVM_RUN`、处理 `kvm_run->exit_reason`

```c
kvm__init()
    kvm__arch_init()
    kvm__init_ram()
    kvm__load_kernel()
```

kvm__init() 主要是创建虚拟机，然后通过 mmap() 申请内存，注册成虚拟机的物理内存，
最后通过 read() 读取内核镜像 bzImage 文件，写入到刚刚申请的内存中

## KVM 的详细实现



## libvirt 使用（可选）



## qemu 实现（可选）



### 参考

Documentation/virt/kvm/api.rst
