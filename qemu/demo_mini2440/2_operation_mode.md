## qemu 运行 mini2440板子

* ubuntu生成虚拟网络tap0

```shell
$ sudo tunctl -u $USER -t tap0
$ sudo ifconfig tap0 192.168.1.125 up
```

* 执行如下命令

```shell
$ ./qemu-system-arm -M mini2440 -serial stdio -mtdblock nand.bin -show-cursor -usb -usbdevice keyboard -usbdevice mouse -net nic,vlan=0 -net tap,vlan=0,ifname=tap0,script=no,downscript=no -monitor telnet::5555,server,nowait
```

## 挂载rootfs的方法有以下两种：

### 1. NFS方式挂载rootfs

* 删除mini2440 qt4版本的rootfs的/etc/init.d/ifconfig-eth0（可选）

```shell
$ mv etc/init.d/ifconfig-eth0 root/
```

* 修改u-boot的bootargs为NFS方式挂载rootfs

```
MINI2440 # nboot kernel

MINI2440 # set bootargs noinitrd root=/dev/nfs rw nfsroot=192.168.1.125:/home/vernon/workplace/qemu-mini2440/output/friendARM2nfs,proto=tcp,nfsvers=3,nolock  ip=192.168.1.126:192.168.1.1::255.255.255.0 console=ttySAC0,115200
或
MINI2440 # set bootargs noinitrd root=/dev/nfs rw nfsroot=192.168.1.125:/home/vernon/workplace/qemu-mini2440/output/buildroot2nfs,proto=tcp,nfsvers=3,nolock  ip=192.168.1.126:192.168.1.1::255.255.255.0 console=ttySAC0,115200

MINI2440 # bootm
```

### 2. 从nand flash中读取rootfs镜像，进行挂载rootfs


* 进入uboot后，执行如下命令

```
MINI2440 # nboot kernel

MINI2440 # set bootargs root=/dev/mtdblock3 rootfstype=jffs2 console=ttySAC0,115200

MINI2440 # bootm
```



