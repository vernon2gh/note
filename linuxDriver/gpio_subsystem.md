## 简述

linux kernel gpio驱动的编写，可以分为四步：

**the integer-based GPIO interface – legacy**

1. 配置设备树dts gpio属性
3. 从设备树dts获得gpio
3. 设置/获得gpio电平

**the descriptor-based GPIO interface - recommended way**

1. 配置设备树dts gpio属性
2. 从设备树dts获得gpio
3. 设置/获得gpio电平
4. 释放gpio

### the integer-based GPIO interface – legacy

#### 1. 配置设备树dts gpio属性

```c
xxx {
    compatible = "xxx,xxx";

    xxx-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>; // 低电平有效
    state = "okay";
};
```

#### 2. 从设备树dts获得gpio

```c
unsigned int xxx_gpio;

xxx_gpio = of_get_named_gpio(np, "xxx-gpios", 0); // 从设备树dts获得xxx-gpios属性的第一个gpio
gpio_direction_output(xxx_gpio, value);           // 默认输出状态, value=1/0，高/低电平
或
gpio_direction_input(xxx_gpio);                   // 默认输入状态
```

#### 3. 设置/获得gpio电平

```c
gpio_set_value(xxx_gpio, value); // value=1/0，高/低电平
gpio_get_value(xxx_gpio);        // 返回值是gpio电平
```

### the descriptor-based GPIO interface - recommended way

#### 1. 配置设备树dts gpio属性

```c
xxx {
    compatible = "xxx,xxx";

    xxx-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>; // 属性名字必须以-gpios为后缀，低电平有效
    state = "okay";
};
```

#### 2. 从设备树dts获得gpio

```c
struct gpio_desc *xxx_gpio;

// 从设备树dts读xxx-gpios属性，默认输出失能状态，高电平
xxx_gpio = gpiod_get(&dev, "xxx", GPIOD_OUT_LOW); 
或
// 从设备树dts读xxx-gpios属性，默认输入状态
xxx_gpio = gpiod_get(&dev, "xxx", GPIOD_IN);
```

#### 3. 设置/获得gpio电平

```c
gpiod_set_value(xxx_gpio, status); // status=1/0, 使能/失能状态，低/高电平
status = gpiod_get_value(xxx_gpio);// 返回值status=1/0, 使能/失能状态, 低/高电平
```

#### 4. 释放gpio

```c
gpiod_put(xxx_gpio);
```

