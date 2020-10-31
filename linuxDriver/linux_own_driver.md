## 简述

linux kernel 自带驱动的编写，都只需要配置设备树dts，一般有led, key等等

* led

  leds节点的compatible属性必须是"gpio-leds"，每一个led作为leds节点的子节点，如led1子节点。

  led1子节点，必须有gpios属性，用于指定控制led的gpio编号

  ```c
  leds {
      compatible = "gpio-leds"; // 必须
      pinctrl-names = "default";
      pinctrl-0 = <&pinctrl_gpio_leds>;
  
      led1{
          label = "led1";                     // 可选
          gpios = <&gpio1 3 GPIO_ACTIVE_LOW>; // 必须
          linux,default-trigger = "heartbeat";// 可选，默认是none模式
          default-state = "on";               // 可选，默认是off
      };
  };
  ```


* key

  keys节点的compatible属性必须是"gpio-keys"，每一个key作为keys节点的子节点，如key0子节点。

  key0子节点，必须有linux,code属性，用于指定上报linux系统的按键值

  必须有gpios属性，用于指定控制key的gpio编号

  ```c
keys {
      compatible = "gpio-keys"; // 必须
      pinctrl-names = "default";
      pinctrl-0 = <&pinctrl_gpio_keys>;
      autorepeat;               // 可选, 按住key时，重复上报标志
  
      key0 {
          label = "KEY0_ENTER";                // 可选
          linux,code = <KEY_ENTER>;            // 必须
          gpios = <&gpio1 18 GPIO_ACTIVE_LOW>; // 必须
      };
  };
  ```

