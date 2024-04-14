# 简介

火焰图工具能够将 perf 采样的 perf.data 以图形化的方式显示出来。

# 下载火焰图工具

```bash
$ git clone https://github.com/brendangregg/FlameGraph.git
```

# 生成火焰图

```bash
$ sudo perf record -g -a                                       ## 生成采样数据 perf.data
$ sudo perf script -i perf.data > perf.unfold                  ## 对 perf.data 进行解析

$ ./FlameGraph/stackcollapse-perf.pl perf.unfold > perf.folded ## 将 perf.unfold 中的符号进行折叠
$ ./FlameGraph/flamegraph.pl perf.folded > perf.svg            ## 生成 svg 图
```
