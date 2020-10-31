## ubuntu PC、linux开发板与windows共享文件
* 在ubuntu PC搭建 nfs 服务器

  ```bash
  $ sudo apt install nfs-kernel-server
  
  $ vim /etc/exports # 加入以下
  <shareFileDir> *(rw,sync,no_root_squash)
  
  $ showmount -e # 测试
  ```

* 在开发板直接打开 nfs

  ```bash
  $ mount -t nfs <ubuntu PC IP>:<shareFileDir> <mountDir>
  ```


* 先在windows中安装NFS服务，直接打开 nfs

  ```powershell
  \\<ubuntu IP>
  ```

