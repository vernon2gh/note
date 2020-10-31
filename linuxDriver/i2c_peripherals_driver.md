## 简述

linux kernel i2c外设驱动的编写，可以分为四步：

**通过i2c_transfer()读写i2c外设寄存器**

1. 配置设备树dts i2c子节点
2. 注册/注销 struct i2c_driver变量
3. 当设备树与驱动匹配后，调用probe函数
4. 读写i2c外设寄存器

**通过regmap API读写i2c外设寄存器**

1. 配置设备树dts i2c子节点**（同上）**
2. 注册/注销 struct i2c_driver变量**（同上）**
3. 当设备树与驱动匹配后，调用probe函数
4. 读写i2c外设寄存器

### 通过i2c_transfer()读写i2c外设寄存器

#### 1. 配置设备树dts i2c子节点

```c
&i2c {
	status = "okay";

	xxx@1e {
		compatible = "xxx,xxx";
		reg = <0x1e>; // i2c外设地址，7bits

		status = "okay";
	};
};
```

#### 2. 注册/注销struct i2c_driver变量

* 定义struct i2c_driver变量

  ```c
  const struct of_device_id xxx_of_match_table[] = {
  	{ .compatible = "xxx,xxx" },
  	{}
  };
  
  const struct i2c_device_id xxx_id_table[] = {
  	//{ .name = "xxx,xxx" },
  	{}
  };
  
  struct i2c_driver xxx_i2c_driver = {
  	.probe = xxx_probe,
  	.remove = xxx_remove,
  	.driver = {
  		.owner = THIS_MODULE,
  		.name = "xxx",
  		.of_match_table = xxx_of_match_table,
  	},
  	/* must add .id_table, otherwise .probe function can not be executed */
  	.id_table = xxx_id_table,
  };
  ```

* 调用相关API注册/注销

  ```c
  i2c_add_driver(&xxx_i2c_driver); // 注册
  i2c_del_driver(&xxx_i2c_driver); // 注销
  或
  module_i2c_driver(xxx_i2c_driver);
  ```

#### 3. 当设备树与驱动匹配后，调用probe函数

设备树与驱动匹配，即设备树dts i2c子节点的compatible属性 与 xxx_of_match_table结构体的.compatible变量相同，调用probe函数

* 通过client->addr获得i2c外设地址
* 通过client->adapter获得i2c控制器
* 通过of_前缀函数获得其它dts资源

```c
int xxx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device dev = client->dev;
	struct device_node *np = dev.of_node;

	xxxDev.client = client;
    
	return 0;
}
```

#### 4. 读写i2c外设寄存器

* 读操作

  ```c
  void xxx_read_regs(u8 reg, u8 *buf, u8 len)
  {
  	struct i2c_msg msg[2];
  
  	msg[0].addr = xxxDev.client->addr; // i2c外设地址
  	msg[0].flags = 0;                  // 写标志
  	msg[0].buf = &reg;                 // 寄存器地址
  	msg[0].len = 1;                    // 寄存器长度
  
  	msg[1].addr = xxxDev.client->addr; // i2c外设地址
  	msg[1].flags = I2C_M_RD;           // 读标志
  	msg[1].buf = buf;                  // 读出的数据缓冲区
  	msg[1].len = len;                  // 读出的数据长度
  
  	i2c_transfer(xxxDev.client->adapter, msg, 2);
  }
  ```

* 写操作

  ```c
  void xxx_write_regs(u8 reg, u8 *buf, u8 len)
  {
  	struct i2c_msg msg;
  	u8 buffer[256];
  
  	msg.addr = xxxDev.client->addr; // i2c外设地址
  	msg.flags = 0;                  // 写标志
  	msg.len = len + 1;              // 写入的数据长度+寄存器长度
  
  	buffer[0] = reg;                // 寄存器地址
  	memcpy(&buffer[1], buf, len);   // 写入的数据缓冲区
  	msg.buf = buffer;               // 寄存器地址+写入的数据缓冲区
  
  	i2c_transfer(xxxDev.client->adapter, &msg, 1);
  }
  ```


### 通过regmap API读写i2c外设寄存器

#### 1. 配置设备树dts i2c子节点（同上）

#### 2. 注册/注销struct i2c_driver变量（同上）

#### 3. 当设备树与驱动匹配后，调用probe函数

设备树与驱动匹配，即设备树dts i2c子节点的compatible属性 与 xxx_of_match_table结构体的.compatible变量相同，调用probe函数

* 通过client->addr获得i2c外设地址
* 通过client->adapter获得i2c控制器
* 通过of_前缀函数获得其它dts资源

```c
int xxx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device dev = client->dev;
	struct device_node *np = dev.of_node;
	struct regmap_config config;
	struct regmap *regmap;

	memset(&config, 0, sizeof(config));
	config.reg_bits = 16; // i2c外设寄存器地址 16bits
	config.val_bits = 8;  // i2c外设寄存器数据 8bits
	regmap = devm_regmap_init_i2c(client, &config); // 初始化regmap

	xxxDev.client = client;
	xxxDev.regmap = regmap;

	return 0;
}
```

#### 4. 读写i2c外设寄存器

* 读操作

  ```c
  int regmap_read(struct regmap *map, unsigned int reg, unsigned int *val)
  ```

* 写操作

  ```c
  int regmap_write(struct regmap *map, unsigned int reg, unsigned int val)
  ```