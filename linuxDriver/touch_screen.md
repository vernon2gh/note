## 简述

linux kernel touch screen driver编写，需要用到i2c子系统、中断子系统、input子系统，这三个子系统前面文档已经介绍过如何操作，此处只介绍input子系统的多点触摸协议，需要五步：

* 手动分配与设置  struct input_dev * 变量
* 注册 input device 
* 向linux系统上报事件
* 注销与释放 input device
* 应用层测试

#### 1. 手动分配与设置  struct input_dev * 变量

```c
struct input_dev *inputDev;

inputDev = input_allocate_device(); // 分配
inputDev->name = "xxx_inputDev";    // 设置input device名字
__set_bit(EV_ABS, inputdev->evbit); // 设置事件类型
input_set_abs_params(inputdev, ABS_MT_POSITION_X, 0, 480, 0, 0);//设置x轴事件码, x轴范围0~480
input_set_abs_params(inputdev, ABS_MT_POSITION_Y, 0, 272, 0, 0);//设置y轴事件码, y轴范围0~272
input_mt_init_slots(inputdev, MAX_SUPPORT_POINTS, 0); // 初始化触摸点个数 MAX_SUPPORT_POINTS
```

#### 2. 注册 input device

```c
input_register_device(inputDev); // 注册
```

#### 3. 向linux系统上报事件

一般在中断处理函数或下半部函数中上报事件

```c
/*
 * 向linux系统上报触摸事件
 *
 * 事件类型: EV_ABS
 * 事件码  : ABS_MT_SLOT、ABS_MT_TRACKING_ID、ABS_MT_POSITION_X、ABS_MT_POSITION_Y
 * 事件值  : slot、touch、x、y, 从触摸屏IC读取得到
 *
 * 事件值解释
 * slot ：代表第几个触摸点
 * touch：代表手指按下/松开 = 1/0
 * x    ：代表触摸点x轴坐标
 * y    ：代表触摸点y轴坐标
 */
input_mt_slot(inputdev, slot);                               // ABS_MT_SLOT
input_mt_report_slot_state(inputdev, MT_TOOL_FINGER, touch); // ABS_MT_TRACKING_ID
input_report_abs(inputdev, ABS_MT_POSITION_X, x);
input_report_abs(inputdev, ABS_MT_POSITION_Y, y);

// 向linux系统上报同步事件
input_sync(inputDev);
```

#### 4. 注销与释放 input device

```c
input_unregister_device(inputDev); // 注销
input_free_device(inputDev);       // 释放
```

#### 5. 应用层测试

通过hexdump测试

```bash
$ hexdump /dev/input/eventx
[编号]   [秒]      [微秒]      [事件类型] [事件码]  [事件值]
0000000 002a 0000 362f 000c    0003     0039    0000 0000
0000010 002a 0000 362f 000c    0003     0035    0025 0000
0000020 002a 0000 362f 000c    0003     0036    0023 0000
0000030 002a 0000 362f 000c    0000     0000    0000 0000

# (x,y) 即(0x25, 0x23)
# 第一行：触摸事件，事件类型EV_ABS=0x3,事件码ABS_MT_SLOT=0x39,事件值0
# 第二行：触摸事件，事件类型EV_ABS=0x3,事件码ABS_MT_POSITION_X=0x35,事件值x坐标 0x25
# 第三行：触摸事件，事件类型EV_ABS=0x3,事件码ABS_MT_POSITION_Y=0x36,事件值y坐标 0x23
# 第四行：同步事件
```

通过tslib测试

```bash
$ vi /etc/profile # 加入以下内容
export TSLIB_TSDEVICE=/dev/input/eventX
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fbx

$ ts_print_mt # 打印坐标(x, y)
$ ts_test_mt  # 图形化显示坐标位置，画图功能
```

**注意：**驱动需要上传手指按下/松开状态，tslib画图功能才能正常使用，否则只能使用图形化显示坐标位置功能