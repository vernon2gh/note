### 简介

> base linux-5.4

bitmap

### 实践

#### 头文件

```c
#include <linux/bitmap.h>
```

#### 与/或/非/异或/与非操作

```c
void bitmap_test(void)
{
	unsigned long dst, src1, src2;
	unsigned int nbits;

	src1 = 0x2345;
	src2 = 0xff;
	nbits = 12;

	bitmap_and(&dst, &src1, &src2, nbits); // 与操作
	printk("bitmap: 0x%lx and 0x%lx\t = 0x%lx\n", src1, src2, dst);

	bitmap_or(&dst, &src1, &src2, nbits);  // 或操作
	printk("bitmap: 0x%lx or 0x%lx\t = 0x%lx\n", src1, src2, dst);

	bitmap_xor(&dst, &src1, &src2, nbits); // 异或操作
	printk("bitmap: 0x%lx xor 0x%lx\t = 0x%lx\n", src1, src2, dst);

	bitmap_complement(&dst, &src2, nbits); // 非操作
	printk("bitmap: 0x%lx complement\t = 0x%lx\n", src2, dst);

	nbits = 16;
	bitmap_andnot(&dst, &src1, &src2, nbits); // 与非操作
	printk("bitmap: 0x%lx andnot 0x%lx\t = 0x%lx\n", src1, src2, dst);
}
```

执行后，如下：

```bash
bitmap: 0x2345 and 0xff  = 0x45
bitmap: 0x2345 or 0xff   = 0x23ff
bitmap: 0x2345 xor 0xff  = 0x23ba
bitmap: 0xff complement  = 0xffffff00
bitmap: 0x2345 andnot 0xff       = 0x2300
```

#### 置位、清零、填充操作

```c
void bitmap_test(void)
{
	unsigned long dst, src1, src2;
	unsigned int nbits, start;

	src2 = 0xff;
	start = 9;
	nbits = 4;
	bitmap_set(&src2, start, nbits); // 置位操作
	printk("bitmap: 0xff set 9~12 bits\t = 0x%lx\n", src2);

	src2 = 0xff;
	start = 5;
	nbits = 3;
	bitmap_clear(&src2, start, nbits); // 清零操作
	printk("bitmap: 0xff clear 5~7 bits\t = 0x%lx\n", src2);

	src2 = 0x20;
	nbits = 32;
	bitmap_fill(&src2, nbits);         // 填充操作
	printk("bitmap: 0x20 fill\t = 0x%lx\n", src2);
}
```

执行后，如下：

```bash
bitmap: 0xff set 9~12 bits       = 0x1eff
bitmap: 0xff clear 5~7 bits      = 0x1f
bitmap: 0x20 fill        = 0xffffffff
```

#### 位移操作

```c
void bitmap_test(void)
{
	unsigned long dst, src1, src2;
	unsigned int nbits, shift;

	src2 = 0x12345678;
	nbits = 32;
	shift = 4;
	bitmap_shift_left(&dst, &src2, shift, nbits); // 左移操作
	printk("bitmap: 0x%lx shift left 4 bits\t = 0x%lx\n", src2, dst);
	bitmap_shift_right(&dst, &src2, shift, nbits); // 右移操作
	printk("bitmap: 0x%lx shift right 4 bits\t = 0x%lx\n", src2, dst);
}
```

执行后，如下：

```bash
bitmap: 0x12345678 shift left 4 bits     = 0x23456780
bitmap: 0x12345678 shift right 4 bits    = 0x1234567
```

#### 遍历操作

```c
void bitmap_test(void)
{
	unsigned long dst, src1, src2, bit;
	unsigned int nbits;

	src2 = 0xf0;
	nbits = 8;
	bit = 0 ;
	for_each_clear_bit(bit, &src2, nbits) // 遍历清零位操作
		printk("bitmap: 0x%lx bit %ld is 0\n", src2, bit);
	for_each_set_bit(bit, &src2, nbits)   // 遍历置位操作
		printk("bitmap: 0x%lx bit %ld is 1\n", src2, bit);
}
```

执行后，如下：

```bash
bitmap: 0xf0 bit 0 is 0
bitmap: 0xf0 bit 1 is 0
bitmap: 0xf0 bit 2 is 0
bitmap: 0xf0 bit 3 is 0
bitmap: 0xf0 bit 4 is 1
bitmap: 0xf0 bit 5 is 1
bitmap: 0xf0 bit 6 is 1
bitmap: 0xf0 bit 7 is 1
```

#### 其余操作

```c
void bitmap_test(void)
{
	unsigned long dst, src1, src2;
	unsigned int nbits;

	src2 = 0xff00;
	nbits = 8;
	if(bitmap_empty(&src2, nbits)) // 判断是否为空
		printk("bitmap: 0x%lx low %d bits is empty\n", src2, nbits);
	else
		printk("bitmap: 0x%lx low %d bits isn't empty\n", src2, nbits);

	src1 = 0x1234;
	src2 = 0xff34;
	nbits = 8;
	if(bitmap_equal(&src1, &src2, nbits)) // 判断是否相等
		printk("bitmap: 0x%lx compare 0x%lx, low %d bits is equal\n",
               src1, src2, nbits);
	else
		printk("bitmap: 0x%lx compare 0x%lx, low %d bits isn't equal\n",
               src1, src2, nbits);
}
```

执行后，如下：

```bash
bitmap: 0xff00 low 8 bits is empty
bitmap: 0x1234 compare 0xff34, low 8 bits is equal
```

