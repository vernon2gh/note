## 简述

linux kernel 定时器的编写，分为五个步骤：

* 定义与初始化定时器
* 设置超时时间与超时处理函数
* 添加定时器
* 超时处理函数
* 删除定时器

#### 1. 定义与初始化定时器

```c
struct timer_list timer;

init_timer(&timer);
```

#### 2. 设置超时时间与超时处理函数

```c
timer.function = timer_function; // 超时处理函数
timer.data = (unsigned long)data;// 传递给 超时处理函数 的参数
timer.expires = jiffies + msecs_to_jiffies(2000); // 超时时间，2秒
```

#### 4. 添加定时器

添加定时器后，定时器就开始计时

```c
add_timer(&timer);
```

#### 3. 超时处理函数

linux kernel 定时器只能执行一次，如果需要重复，需要在 超时处理函数 调用mod_timer()重新设置超时时间并启动即可

```c
void timer_function(unsigned long data)
{
    mod_timer(&timer, jiffies + msecs_to_jiffies(2000)); // 修改超时时间，2秒
}
```

#### 4. 删除定时器

```c
del_timer(&timer);
```

