为了进一步提升指令执行效率，编译器与 CPU 都提供各自的优化策略。

编译器提供 `likely() and unlikely()` 函数给软件显式使用，用于告诉编译器哪一个分支执行概率大，从而帮忙编译器更好地优化代码执行，来提升指令执行效率。

CPU 提供了 speculative execution（推测执行），这是硬件内部优化技术，无需软件进行任何操作。主要动作是 CPU 尝试猜测下一步指令并预先执行，从而在需要时可以更快地提供结果，来提升指令执行效率。

这里有一个问题，CPU  speculative execution 是来自于未经验证的数据，可能导致数组越界访问内存区域（如果这块内存区域刚好存储密码等重要信息），虽然该分支最终不会被实际执行，但是由于 L1/L2/L3 cache 的原因，这些预先执行指令得到的数据会存在 cache 中，然后就能够利用 cache 执行速度比普通内存快的特征来判断，得到这些缓存在 cache 中的数据。这也就是著名的 Meltdown/Spectre 安全漏洞。

下面是一个伪代码：

```c
foo();

if (index < size)
    val = array[index];
```

当 CPU 执行 `foo()` 时，CPU speculative execution 可能预先执行 `val = array[index]` ，这样即使后面真正执行 `if (index < size)` 无法成立， 但是 `array[index]` 值已经缓存在 cache 中，然后就能够利用 cache 执行速度比普通内存快的特征来判断，得到这些缓存在 cache 中的数据。

于是我们需要告诉 CPU 硬件不要做分支预测或者说控制 `index` 不要越界，这就是 `array_index_nospec()` 的功能。

修改后的伪代码：

```
foo();

if (index < size) {
    index = array_index_nospec(index, size);
    val = array[index];
}
```

修改后，当 CPU speculative execution 时，`array_index_nospec()` 能够保证 index 是一个合法值，从 而 不会导致非法 `array[index]` 值被缓存在 cache 中。

还有其它一些防止 CPU speculative execution 带来的额外问题，可以参考 `include/linux/nospec.h`。

