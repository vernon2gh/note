## 如何获得ssh登录用户名和密码？

### 简要说明

在Linux系统中执行某些程序时，在执行前首先要对启动它的用户进行认证，符合一定的要求之后才允许执行，其中认证过程是通过PAM（Pluggable Authentication Modules）来执行。



比如 ssh，ssh执行认证过程，通过PAM调用配置文件`/etc/pam.d/sshd`进行认证，从而调用本地的认证模块，认证模块放置在`/lib/security`，以加载动态库的形式进行。

* PAM配置文件的格式，如`/etc/pam.d/sshd`

```bash
auth optional record.so
```

第一列代表模块类型，`auth`对用户的身份进行识别。如：提示用户输入密码等

第二列代表控制标记，`optional`表示即使本行指定的模块验证失败，也允许用户接受应用程序提供的服务，一般返回PAM_IGNORE(忽略)

第三列代表模块路径，如果模块保存在默认存储目录`/lib/security`，可以直接写 模块名字

第四列代表模块参数，传递给模块的参数，此处省略代表没有参数

* 模块类型`auth`需要在模块源码中实现如下服务函数接口

```c
PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
```

### 测试环境

目前是基于ubuntu18.04进行测试，采用默认GNOME桌面环境。

### 安装相关软件

```bash
$ sudo apt install ssh
$ sudo apt install libpam0g libpam0g-dev ## （可选）编译软件时必须安装
```

### 编译软件

通过[Makefile](../../resources/code/c/pam/Makefile)编译安装[源码](../../resources/code/c/pam/record.c)

```bash
$ sudo su - root
$ cd pam/
$ ls
Makefile  record.c

$ make         ## 编译生成record.so
$ make install ## 安装record.so并且配置环境
```

具体源码细节，请看源码注释。

### 参考网址

https://zhuanlan.zhihu.com/p/146024506
