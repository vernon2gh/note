## 简述

linux kernel IRQ驱动的编写，可以分为四步，但是中断处理函数的下半部处理有 tasklet context, workqueue context等等，所以此处分开讲解：

**tasklet context**

1. 配置设备树dts
3. 从设备树dts获得与申请中断号
3. 设置中断处理函数
4. 下半部处理（tasklet context）
5. 释放中断号

**workqueue context**

1. 配置设备树dts**（同上）**
2. 从设备树dts获得与申请中断号**（同上）**
3. 设置中断处理函数
4. 下半部处理（workqueue context）
5. 释放中断号**（同上）**

### tasklet context

#### 1. 配置设备树dts

```c
xxx {
    compatible = "xxx,xxx";

    // GPIO1_18对应的中断号, 下降沿触发
    interrupt-parent = <&gpio1>;
    interrupts = <18 IRQ_TYPE_EDGE_FALLING>;
    state = "okay";
};
```

#### 2. 从设备树dts获得与申请中断号

* 获得中断号

  ```c
  unsigned int xxx_irq;
  
  xxx_irq = irq_of_parse_and_map(np, 0); // 0代表interrupts属性的第一个中断号
  ```

* 申请中断号

  ```c
  unsigned int ret;
  
  /*
   * 中断号               : xxx_irq
   * 中断处理函数          ：xxx_irq_handler()
   * 触发条件             ：IRQF_TRIGGER_FALLING, 下降沿触发
   * 中断名               ：xxxIRQ
   * 传递给中断处理函数的参数：xxxDev指针
   */
ret = request_irq(xxx_irq, xxx_irq_handler, IRQF_TRIGGER_FALLING, "xxxIRQ", &xxxDev);
  if(ret != 0) {
      dev_err(&dev, "request irq failed, ret %d\n", ret);
      return ret;
  }
  或
  // 中断线程化，中断处理函数与线程函数是同一优先级，可以相互竞争CPU资源
  ret = request_threaded_irq(xxx_irq, NULL, xxx_irq_handler,
      IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "xxxIRQ", &xxxDev);
  if(ret != 0) {
      dev_err(&dev, "request irq failed, ret %d\n", ret);
      return ret;
  }
  ```
  

#### 3. 设置中断处理函数 

```c
static irqreturn_t xxx_irq_handler(int irq, void *dev)
{
    struct xxx_dev *p_xxxDev = (struct xxx_dev *)dev;

    tasklet_schedule(&xxx_irq_tasklet); // 调用tasklet_func()

    return IRQ_HANDLED;
}
```

#### 4. 下半部处理（tasklet context）

```c
void tasklet_func(unsigned long data)
{
    struct xxx_dev *p_xxxDev = (struct xxx_dev *)data;

}

/*
 * 定义xxx_irq_tasklet
 * 传递给tasklet_func()的参数是xxxDev指针
 */
DECLARE_TASKLET(xxx_irq_tasklet, tasklet_func, (unsigned long)&xxxDev);
```

#### 5. 释放中断号

```c
/*
 * 中断号               : xxx_irq
 * 传递给中断处理函数的参数：xxxDev指针
 */
free_irq(xxx_irq, &xxxDev);
```

### workqueue context

#### 1. 配置设备树dts（同上）

#### 2. 从设备树dts获得与申请中断号（同上）

#### 3. 设置中断处理函数

```c
static irqreturn_t xxx_irq_handler(int irq, void *dev)
{
    struct xxx_dev *p_xxxDev = (struct xxx_dev *)dev;

    schedule_work(&xxx_irq_work); // 调用work_func()
    
    return IRQ_HANDLED;
}
```

#### 4. 下半部处理（workqueue context）

```c
void work_func(struct work_struct *work)
{

}

// 定义xxx_irq_work
DECLARE_WORK(xxx_irq_work, work_func);
```

#### 5. 释放中断号（同上）

