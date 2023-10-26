```bash
$ cat /proc/sys/vm/min_free_kbytes
67584
```

`min_free_kbytes` 代表内存回收的 min 水位，当空闲内存低于 min 水位时，
执行直接内存回收流程。默认 low 水位等于 `min 水位 + 0.25 * min 水位`，
默认 high 水位等于 `min 水位 + 2 * 0.25 * min 水位`

```bash
$ cat /proc/sys/vm/watermark_scale_factor
10
```

`watermark_scale_factor` 代表 `low, high 水位` 的比例，当
`系统所有内存大小 * watermark_scale_factor / 10000` 大于 `0.25 * min 水位`，
将 `min 水位 + 系统所有内存大小 * watermark_scale_factor / 10000` 设置为新 low 水位,
将 `min 水位 + 2 * 系统所有内存大小 * watermark_scale_factor / 10000` 设置为新 high 水位
