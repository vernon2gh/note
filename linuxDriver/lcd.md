## 1. 简述

linux kernel lcd驱动

imx6ull的lcd控制器驱动已经由NXP编写好了，我们只需要在设备树dts里面配置lcd控制器的参数，就可以初始化外设lcd，如下

```c
&lcdif {
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_lcdif_dat
    	&pinctrl_lcdif_ctrl>;
    display = <&display0>;
    status = "okay";

    display0: display {
        bits-per-pixel = <24>;         // 每一个像素点由24位组成
        bus-width = <24>;              // lcd需要数据总线宽度，24位

        display-timings {
            native-mode = <&timing0>;
            timing0: timing0 {
                clock-frequency = <9000000>; // LCD像素时钟，9MHZ
                hactive = <480>;             // 水平方向的分辨率
                vactive = <272>;             // 垂直方向的分辨率
                hfront-porch = <5>;
                hback-porch = <40>;
                hsync-len = <1>;
                vback-porch = <8>;
                vfront-porch = <8>;
                vsync-len = <1>;

                hsync-active = <0>;         // hsync 极性
                vsync-active = <0>;         // vsync 极性
                de-active = <1>;            // de 极性
                pixelclk-active = <0>;      // pixelclk 极性
            };
        };
    };
};
```

lcd还需要通过PWM控制背光强度，一般PWM驱动已经由NXP编写好了，我们只需要在设备树dts里面配置PWM即可

```c
&pwm1 {
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_pwm1>;
    status = "okay";
};
```

pwm1使能后，linux kernel如何知道lcd通过pwm1控制背光强度？所以我们还需要一个blacklight节点将lcd与pwm连接起来，如下：

```c
backlight {
    compatible = "pwm-backlight";
    pwms = <&pwm1 0 5000000>;                    // PWM1 5KHZ
    brightness-levels = <0 4 8 16 32 64 128 255>;// 背光级别 0~7,对应pwm占空比 0%~100%
    default-brightness-level = <6>;              // 默认背光级别为6
    status = "okay";
};
```

到此，lcd驱动就完成了。

### 2. 测试

#### 2.1 基本测试--使能 Linux logo 显示

```
Device Drivers  --->
	Graphics support  --->
		[*] Bootup logo  --->
			[*]   Standard black and white Linux logo
			[*]   Standard 16-color Linux logo
			[*]   Standard 224-color Linux logo
```

如果 LCD 驱动工作正常的话，在 LCD 屏幕左上角出现一个彩色的小企鹅 logo，屏幕背景色为黑色

#### 2.2  设置LCD作为终端控制台

* 设置uboot中的bootargs

  ```
  setenv bootargs console=tty0 console=ttymxc0,115200 [xxx]
  ```

  此处有两个终端，一个LCD终端（tty0），一个串口终端（ttymxc0），同时都有linux kernel log显示。

* 设置终端启动bash，如tty0或ttymxc0

  ```bash
  $ cd buildroot
  $ make menuconfig
  System configuration  --->
  	[*] Run a getty (login prompt) after boot  --->
  		(tty0) TTY port
  		或
  		(ttymxc0) TTY port
  ```

#### 2.3 LCD背光调节

由设备树dts的backlight节点，可知有0~7个背光亮度级别（即brightness-levels属性），默认背光亮度级别是6（即default-brightness-level属性），我们可以通过如下命令对背光亮度进行调节

```bash
$ cd /sys/class/backlight/backlight
$ cat max_brightness  # 当前支持最大的背光亮度级别，由dts的brightness-levels决定
7
$ cat brightness      # 默认当前背光亮度级别，由dts的default-brightness-level决定
6
$ echo 0 > brightness # 背光亮度级别等于0，相当于0%，灭屏
$ echo 7 > brightness # 背光亮度级别等于7，相当于100%，最亮
```

