# Linux内核中的常见锁

## 自旋锁

### 普通自旋锁 spinlock

相关结构体：

```c
spinlock: struct spinlock(
	union {
        struct raw spinlock rlock:
    }
} spinlock t:

struct raw_ spinlock {
    arch spinlock_t raw lock;
}
```

* 获取锁的过程(即上锁的过程)是自旋(忙等)的，**不会引起睡眠和调度**

* 持有自旋锁的临界区中不允许调度和睡眠，自旋锁的加锁操作会禁止抢占，解锁操作时再恢复抢占

  自旋锁既可以用于进程上下文，又可以用于中断上下文

* 自旋锁的主要用途是多处理器之间的并发控制，**适用于锁竞争不太激烈的场景**

  如果锁竞争非常激烈，那么大量的时间会浪费在加锁自旋中，导致整体性能下降

相关API：

```c
DEFINE_SPINLOCK(lock) // 静态定义一个名为lock的自旋锁
spin_lock_init(lock)  // 自旋锁初始化(设置为未锁状态)

spin_lock(lock)                // 加锁操作，加锁成功后返回，否则一直自旋(忙等)
spin_lock_irqsave(lock, flags) // 加锁操作，并关闭硬中断
spin_lock_bh(lock)             // 加锁操作，并关闭软中断

spin_unlock(lock)                   // 解锁操作，解锁过程无竟争，因此必然会成功
spin_unlock_irqrestore(lock, flags) // 解锁操作，并打开硬中断
spin_unlock_bh(lock)                // 解锁操作，并打开软中断
```

### 读写自旋锁 rwlock_t

普通自旋锁的缺点:

* 对所有的竞争者不做区分

* 很多情况下，有些竞争者并不会修改共享资源(只读不写)

* 普通自旋锁总是会限制只有一个内核路径持有锁，而实际上这种限制是没有必要的

读写自旋锁的改进:

* 允许多个读者同时持有读锁(允许多个读者同时进入读临界区)

* 只允许一个写者同时持有写锁(只允许一个写者同时进入写临界区)

* 不允许读者和写者同时持有锁

* 与普通自旋锁相比，读写自旋锁**更适合读者多，写者少的应用场景**

相关API：

```c
DEFINE_RWLOCK(lock) // 静态定义一个名为lock的自旋锁
rwlock_init(lock)   // 自旋锁初始化(设置为未锁状态)

read_lock(lock)     // 加读锁操作，加锁成功后返回，否则一直自旋
write_lock(lock)    // 加写锁操作，加锁成功后返回，否则一直自旋
read_unlock(lock)   // 解读锁操作，解锁过程无竟争，因此必然会成功
wite_unlock(lock)   // 解写锁操作，解锁过程无竟争，因此必然会成功
```

## 信号量

* Linux内核中应用最广泛的同步方式: 自旋锁、信号量( semaphore)

* 自旋锁和信号量是一种互补的关系，它们有各自适用的场景

* 信号量可以是多值的，当其用作二值信号量时，类似于锁 : 一个值代表未锁，另一个值代表已锁

* 工作原理与自旋锁相反

  * 获取锁的过程中，若不能立即得到锁，就**会发生调度，转入睡眠**

  * 另外的内核执行路径释放锁时，唤醒等待该锁的执行路径

* 自旋锁的使用限制及信号量的解决方法

  * 持有自旋锁的临界区不允许调度和睡眠 : 竟争激烈时整体性能不好

  * 信号量解决了以上两个问题:

    * 锁的竟争者不是忙等，信号量的临界区允许调度和睡眠而不会导致死锁

    * 锁的竞争者会转入睡眠，从而让出CPU资源给别的内核执行路径，因此对锁的竞争不会影响整体性能

  * 信号量的缺点:

    * 中断上下文要求整体运行时间可预测(不能太长)，而信号量临界区可能发生调度，因此**不能用于中断上下文(只能用于进程上下文)**

    * 如果抢锁的过程很短，那么用信号量并不合算，因为进程睡眠加上唤醒的代价太大，消耗的CPU资源可能远远大于短时间的忙等

### 普通信号量

相关结构体：

```c
 struct semaphore {
     raw_spinlock_t lock;
     unsigned int count;
     struct list_head wait_list;
 };
```

* count标识信号量的状态 : 值为0表示忙(已锁)，值为正代表自由(未锁，允许竞争者进入临界区)

* count的初值就是最大允许进入临界区的进程数目，初值为1的信号量就是二值信号量

* 二值信号量类似于一个普通的锁，而多值信号量类似于一个允许一定并发性的锁

* wait _list字段是当信号量为忙时，所有等待信号量的进程列表，而lock则是保护wait_list的自旋锁

相关API：

```c
DEFINE_SEMAPHORE(sem) 　　　　　　　　　　　　　　　　// 静态定义一个名为sem信号量
void sema_ init(struct semaphore *sem， int val) // 初始化一个信号量sem，计数器初值为val

// 减少信号量sem的计数器(类似于获取锁)
// 如果失败(计数器己经是0)，那么转入睡眠(状态为IASK_ UNINTERRUPTIBLE，不会被任何信号唤醒)并把当其进程挂到wait_list; 被唤醒后继续尝试获取锁
void down(struct semaphore *sem)
// 增加信号量sem的计数器(类似于释放锁)，然后唤醒wait_list里面的第一个进程(如果有的话)
void up(struct semaphore *sem)
```

### 读写信号量

读写信号量的引入原因类似于读写自旋锁，是为了区分不同的竟争者(读者和写者)，以便允许读者共享而写者互斥

相关结构体：

```c
struct rw_semaphore {
	long count;
    struct list_head wait_list;
    raw_spinlock_t wait_lock;
#ifdef CONFIG_RWSEM_SPIN_ON_OWNER
    struct optimistic_spin_queue osq;
    struct task_struct *owner;
#endif
}
```

主要字段 count、 wait_list 和 wait_lock的含义与普通信号量基本相同，owner指向写者进程

相关API：

```c
DECLARE_RWSEN(sem) // 静态声明一个名为sem信号量
init_rwsen(sem)    // 初始化一个信号量sem

down_read(struct rw_semaphore *sem) // 读者减少信号量sem的计数器(类似于获取锁)
down_write(struct rw_semaphore *sem)// 写者减少信号量sem的计数器(类似于获取锁)
up_read(struct rw_semaphore *sem)   // 读者增加信号量sem的计数器(类似于释放锁)
up_write(struct rw_semaphore *sem)  // 写者增加信号量sem的计数器(类似于释放锁)
```

### 互斥量

互斥量是二值信号量

相关结构体：

```c
struct mutex {
    atomic_t count;
    spinlock_t wait_lock;
    struct list_head wait_list;
#ifdef CONFIG_MUTEX_SPIN_ON_OWNER
    struct task_struct *owner;
    struct optimistic_spin_queue osq;
#endif
};
```

在数据结构上与信号量几乎相同

相关API：

互斥量提供了一套新的API，这套API专为二值的互斥量优化

```c
DEFINE_MUTEX(mutex) // 静态定义一个名为 mutex的互斥量
mutex_init(mutex)   // 初始化一个互斥量 mutex，初始状态为未锁

// 对互斥量加锁，如果失败，那么转入睡眠(状态为 TASK_UNINTERRUPTIBLE，不会被任何信号唤醒)并将进程挂到wait_list
void mutex_lock(struct mutex *lock)
// 对互斥量解锁，然后唤醒wait_list里面的第一个进程(如果有的话)
void mutex_unlock(struct mutex *lock)
```

## 死锁原理

典型例子:

* spinlock, rwlock, mutex 在 linux内核中是**不能递归**，如果产生递归会发生死锁

  例如: 调用spinlock(A)，在本cpu上发生中断，中断上下文也调用到 spinlock(A)

* 多个CPU相互等待lock A/B

  --------CPU1------------------------------CPU2---------

  获取 lock A -> ok ----------------- 获取 lock B -> ok
  
  获取 lock B -> spin --------------- 获取 lock A -> spin
