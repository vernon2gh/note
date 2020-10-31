## 简述

linux kernel spi外设驱动的编写，可以分为四步：

**通过spi_sync()读写spi外设寄存器**

1. 配置设备树dts spi子节点
2. 注册/注销 struct spi_driver 变量
3. 当设备树与驱动匹配后，调用probe函数
4. 通过spi_sync()读写spi外设寄存器

**通过regmap API读写spi外设寄存器**

1. 配置设备树dts spi子节点**（同上）**
2. 注册/注销 struct spi_driver 变量**（同上）**
3. 当设备树与驱动匹配后，调用probe函数
4. 通过regmap API读写spi外设寄存器

### 通过spi_sync()读写spi外设寄存器

#### 1. 配置设备树dts spi子节点

```c
&spi {
    cs-gpio = <&gpio1 20 GPIO_ACTIVE_LOW>;
    status = "okay";

    xxx {
        compatible = "xxx,xxx";
        status = "okay";
    };
};
```

#### 2. 注册/注销 struct spi_driver 变量

* 定义 struct spi_driver 变量

  ```c
  const struct of_device_id xxx_of_match_table[] = {
      { .compatible = "xxx,xxx", },
  };
  
  const struct spi_device_id xxx_id_table[] = {
      { .name = "xxx,xxx", },
  };
  
  struct spi_driver xxx_spi_drv = {
      .probe  = xxx_probe,
      .remove = xxx_remove,
      .driver = {
          .owner = THIS_MODULE,
          .name = "xxx",
          .of_match_table = xxx_of_match_table,
      },
      .id_table = xxx_id_table,
  };
  ```

* 调用相关API注册/注销

  ```c
  spi_register_driver(&xxx_spi_drv);  // 注册
  spi_unregister_driver(&xxx_spi_drv);// 注销
  ```

#### 3. 当设备树与驱动匹配后，调用probe函数

设备树与驱动匹配，即设备树dts spi子节点的compatible属性 与 xxx_of_match_table结构体的.compatible变量相同，调用probe函数

```c
int xxx_probe(struct spi_device *spi)
{
    struct device dev = spi->dev;
    struct device_node *np = dev.of_node;
    unsigned int cs_gpio;

    cs_gpio = of_get_named_gpio(np->parent, "cs-gpio", 0); // 从dts获得CS引脚
    gpio_direction_output(cs_gpio, 1);                     // 将CS引脚设置为输出，高电平状态

    spi->mode = SPI_MODE_0; /*MODE0，CPOL=0，CPHA=0*/
    spi_setup(spi);

    xxxDev.spi = spi;
    xxxDev.cs_gpio = cs_gpio;

    return 0;
}
```

#### 4. 通过spi_sync()读写spi外设寄存器

* 读操作

  ```c
  void xxx_read_regs(u8 reg, void *buf, int len)
  {
      struct spi_device *spi = xxxDev.spi;
      struct spi_message m;
      struct spi_transfer *t;
      u8 spiReg = reg | 0x80;            // 寄存器地址+读标志
  
      t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
      gpio_set_value(xxxDev.cs_gpio, 0); // 使能CS引脚
  
      t->tx_buf = &spiReg;
      t->len = 1;
      spi_message_init(&m);
      spi_message_add_tail(t, &m);
      spi_sync(spi, &m);
  
      t->rx_buf = buf;                    // 读出的数据缓冲区
      t->len = len;                       // 读出的数据长度
      spi_message_init(&m);
      spi_message_add_tail(t, &m);
      spi_sync(spi, &m);
  
      gpio_set_value(xxxDev.cs_gpio, 1);  // 失能CS引脚
      kfree(t);
  }
  ```

* 写操作

  ```c
  void xxx_write_regs(u8 reg, void *buf, int len)
  {
      struct spi_device *spi = xxxDev.spi;
      struct spi_message m;
      struct spi_transfer *t;
      u8 spiReg = reg & (~0x80);        // 寄存器地址+写标志
  
      t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
      gpio_set_value(xxxDev.cs_gpio, 0);// 使能CS引脚
  
      t->tx_buf = &spiReg;
      t->len = 1;
      spi_message_init(&m);
      spi_message_add_tail(t, &m);
      spi_sync(spi, &m);
  
      t->tx_buf = buf;                 // 写入的数据缓冲区
      t->len = len;                    // 写入的数据长度
      spi_message_init(&m);
      spi_message_add_tail(t, &m);
      spi_sync(spi, &m);
  
      gpio_set_value(xxxDev.cs_gpio, 1);// 失能CS引脚
      kfree(t);
  }
  ```

### 通过regmap API读写spi外设寄存器

**前提：**spi控制器自动使能/失能CS引脚 或 spi控制器驱动手动使能/失能CS引脚，即 不需要spi外设驱动使能/失能CS引脚

#### 1. 配置设备树dts spi子节点（同上）

#### 2. 注册/注销 struct spi_driver 变量（同上）

#### 3. 当设备树与驱动匹配后，调用probe函数

设备树与驱动匹配，即设备树dts spi子节点的compatible属性 与 xxx_of_match_table结构体的.compatible变量相同，调用probe函数

```c
int xxx_probe(struct spi_device *spi)
{
    struct device dev = spi->dev;
    struct regmap_config config;
    struct regmap *regmap;

    spi->mode = SPI_MODE_0; /*MODE0，CPOL=0，CPHA=0*/
    spi_setup(spi);

    memset(&config, 0, sizeof(config));
    config.reg_bits = 8; // spi外设寄存器地址 8bits
    config.val_bits = 8; // spi外设寄存器数据 8bits
    regmap = devm_regmap_init_spi(spi, &config); // 初始化regmap

    xxxDev.spi = spi;
    xxxDev.regmap = regmap;

    return 0;
}
```

#### 4. 通过regmap API读写spi外设寄存器

* 读操作

  ```c
  int regmap_read(struct regmap *map, unsigned int reg, unsigned int *val)
  ```

* 写操作

  ```c
  int regmap_write(struct regmap *map, unsigned int reg, unsigned int val)
  ```