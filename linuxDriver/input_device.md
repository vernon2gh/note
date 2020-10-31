## 简述

linux kernel input device编写，需要五步：

* 手动分配与设置  struct input_dev * 变量
* 注册 input device
* 向linux系统上报事件
* 注销与释放 input device
* 应用层测试

#### 1. 手动分配与设置  struct input_dev * 变量

```c
struct input_dev *xxx_inputDev;

xxx_inputDev = input_allocate_device(); // 分配
xxx_inputDev->name = "xxx_inputDev";    // 设置input device名字
__set_bit(EV_KEY, xxx_inputDev->evbit); // 设置事件类型
__set_bit(KEY_0,  xxx_inputDev->keybit);// 设置事件码
```

#### 2. 注册 input device

```c
input_register_device(xxx_inputDev); // 注册
```

#### 3. 向linux系统上报事件

一般在中断处理函数或下半部函数中上报事件

```c
/*
 * 向linux系统上报按键事件
 *
 * 事件类型: EV_KEY
 * 事件码  : KEY_0
 * 事件值  : value, 1 or 0
 */
input_report_key(xxx_inputDev, KEY_0, value);

// 向linux系统上报同步事件
input_sync(xxx_inputDev);
```

#### 4. 注销与释放 input device

```c
input_unregister_device(xxx_inputDev); // 注销
input_free_device(xxx_inputDev);       // 释放
```

#### 5. 应用层测试

```bash
$ hexdump /dev/input/eventx
[编号]   [秒]      [微秒]      [事件类型] [事件码]  [事件值]
0000000 0c41 0000 d7cd 000c    0001     000b   0001 0000
0000010 0c41 0000 d7cd 000c    0000     0000   0000 0000
0000020 0c42 0000 54bb 0000    0001     000b   0000 0000
0000030 0c42 0000 54bb 0000    0000     0000   0000 0000

# 第一行：按键事件(按下)，事件类型EV_KEY=0x1,事件码KEY_0=0xb,事件值value=0x1
# 第二行：同步事件
# 第三行：按键事件(松开)，事件类型EV_KEY=0x1,事件码KEY_0=0xb,事件值value=0x0
# 第四行：同步事件
```

