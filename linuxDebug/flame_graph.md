# 简介

火焰图工具能够将 perf 采样的 perf.data 以图形化的方式显示出来。

# 下载火焰图工具

## [FlameGraph](https://github.com/brendangregg/FlameGraph)

```bash
$ git clone https://github.com/brendangregg/FlameGraph.git
```

## [flamelens](https://github.com/YS-L/flamelens)

```bash
$ cargo install flamelens --locked
$ cargo install inferno
```

# 生成火焰图

```bash
## 生成采样数据 perf.data
$ perf record -g -a

## 先使用 perf script 将 perf.data 解析成 perf.unfold，
## 然后再使用 stackcollapse-perf 将符号进行折叠生成 perf.folded，
## 最后使用 flamegraph 生成 perf.svg，并且使用浏览器打开火焰图
$ perf script | ./stackcollapse-perf.pl | ./flamegraph.pl > perf.svg

## 先使用 perf script 将 perf.data 解析成 perf.unfold，
## 然后再使用 inferno-collapse-perf 将符号进行折叠生成 perf.folded，
## 最后直接使用 flamelens 打开火焰图
$ perf script | inferno-collapse-perf | flamelens
```
