# [bcc](https://github.com/iovisor/bcc)

## 最小 demo

[execve.py](https://github.com/vernon2gh/app_and_module/blob/main/ebpf/bcc/execve.py)

当用户空间执行进程时（如：`date` 查看时间），调用系统调用 `execve()`，触发打印 `Hello bcc！`

## 安装依赖+执行

```bash
$ apt install python3-bpfcc

$ python execve.py
```

## 参考

* [Documentation](https://github.com/iovisor/bcc/blob/master/docs/tutorial_bcc_python_developer.md)
* [examples](https://github.com/iovisor/bcc/tree/master/examples)

# libbpf (tools/lib/bpf)

## 最小 demo

[execve.bpf.c](https://github.com/vernon2gh/app_and_module/blob/main/ebpf/libbpf/execve.bpf.c)
[execve.c](https://github.com/vernon2gh/app_and_module/blob/main/ebpf/libbpf/execve.c)

当用户空间执行进程时（如：`date` 查看时间），调用系统调用 `execve()`，触发打印 `Hello libbpf!`

## 编译+执行

```bash
$ cd path/to/linux/kernel

$ make headers_install
$ make M=samples/bpf

$ ./samples/bpf/execve
$ cat /sys/kernel/tracing/trace_pipe
```

## 参考

* Documentation/bpf/libbpf
* samples/bpf/README.rst
* samples/bpf

# [libbpf-bootstrap](https://github.com/libbpf/libbpf-bootstrap)

## 最小 demo

[execve.bpf.c](https://github.com/vernon2gh/app_and_module/blob/main/ebpf/libbpf-bootstrap/execve.bpf.c)
[execve.c](https://github.com/vernon2gh/app_and_module/blob/main/ebpf/libbpf-bootstrap/execve.c)

当用户空间执行进程时（如：`date` 查看时间），调用系统调用 `execve()`，触发打印 `Hello libbpf-bootstrap!`

## 编译+执行

```bash
$ git clone --recurse-submodules https://github.com/libbpf/libbpf-bootstrap
$ cd libbpf-bootstrap

## C Examples + Makefile build
$ make -C examples/c

$ ./examples/c/execve
$ cat /sys/kernel/tracing/trace_pipe
```

## 参考

* [Documentation](https://github.com/libbpf/libbpf-bootstrap/blob/master/README.md)
* [examples](https://github.com/libbpf/libbpf-bootstrap/tree/master/examples)

# [bpftrace](./bpftrace.md)
