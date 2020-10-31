## 简述

linux kernel 避免共享资源冲突的方法有4种，如原子操作、自旋锁、信号量和互斥体。

* 原子操作

  ```c
  atomic_t atomic = ATOMIC_INIT(0); // 定义与初始化, atomic=0
  
  atomic_set(&atomic, 20);          // 相当于 atomic=20
  atomic_inc(&atomic);              // 相当于 atomic+=1, 21
  pr_debug("%s: test == %d\n", __func__, atomic_read(&atomic));
  ```

* 自旋锁

  如果得不到自旋锁，进程不能进入睡眠状态，一直在原地等待

  ```c
  DEFINE_SPINLOCK(spinlock); // 定义与初始化
  
  unsigned long flags;
  spin_lock_irqsave(&spinlock, flags);
  pr_debug("%s: spinlock function\n", __func__); // can not sleep
  spin_unlock_irqrestore(&spinlock, flags);
  ```

* 信号量

  如果得不到信号量，进程进入睡眠状态，等待唤醒

  ```c
  struct semaphore sem; // 定义
  
  sema_init(&sem, 10);  // 初始化, 信号量有10个，即同时允许10个进程访问共享资源
  down(&sem);           // 信号量减一
  pr_debug("%s: semaphore function\n", __func__); // can sleep
  up(&sem);             // 信号量加一
  ```

* 互斥体

  如果得不到互斥体，进程进入睡眠状态，等待唤醒
  
  注意：互斥体与信号量为1时，效果是一样的
  
  ```c
  DEFINE_MUTEX(mutexlock); // 定义与初始化
  
  mutex_lock(&mutexlock);
  pr_debug("%s: mutexlock function\n", __func__); // can sleep
  mutex_unlock(&mutexlock);
  ```