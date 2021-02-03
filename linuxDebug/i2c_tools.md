## 简介
i2c-toos 工具集有 i2cdetect、i2cdump、i2cget、i2cset、i2c-stub-from-dump、i2ctransfer

i2cdump、i2cget、i2cset，默认设置读取8位的寄存器地址

i2ctransfer ，可以对i2c设备指定定长度进行读写操作，比如寄存器地址是16位


## 使用 i2ctransfer 读写 imx477寄存器
### 前提

* 启动 imx477 摄像头

* imx477 挂在 i2c-30 总线上，地址为 0x10，寄存器地址为 16 位


### 检测 imx477 是否存在
NOTE : 当驱动里占用此设备时，地址显示为UU，否则显示具体值地址，比如0x10

* 执行如下命令：

```bash
$ sudo i2cdetect -y -r 30
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: UU -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: UU -- -- -- -- -- -- --
```


* 解析：

| 参数 | 说明             |
| ---- | ---------------- |
| 30   | 表示 i2c-30 总线 |


### 读取 imx477 寄存器
* 执行如下命令：

```bash
$ sudo i2ctransfer -f -y 30 w2@0x10 0x03 0x4D r1
0xb4
```


* 解析：

| 参数      | 说明                         |
| --------- | ---------------------------- |
| 30        | 表示 i2c-30 总线             |
| w2        | 表示写两个字节               |
| 0x10      | 表示 i2c设备地址             |
| 0x03 0x4D | 表示 imx477寄存器地址 0x034D |
| r1        | 表示读取一个字节             |


### 写入 imx477 寄存器
* 执行如下命令：

```bash
$ sudo i2ctransfer -f -y 30 w3@0x10 0x03 0x4D 0xb0
$ sudo i2ctransfer -f -y 30 w2@0x10 0x03 0x4D r1
0xb0
```


* 解析：

| 参数           | 说明                                           |
| -------------- | ---------------------------------------------- |
| 30             | 表示 i2c-30 总线                               |
| w3             | 表示写三个字节                                 |
| 0x10           | 表示 i2c设备地址                               |
| 0x03 0x4D 0xb0 | 表示 向imx477寄存器地址 0x034D处，写入0xb0数据 |
