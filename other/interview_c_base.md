## const

**第一种情况：**

```c
const int a = 8;
int const b = 9;

// a = 7; /* ERROR */
```

这两种写法是一样的，表示a和b都是常量，不可改变，所以一定要给变量初始化，否则之后就不能再进行赋值了

**第二种情况：**

```c
const int *p;
or
int const *p;

int a = 9;
int b = 10;

p = &a;

// *p = 8; /* ERROR */
a = 8;
p = &b;
```

常量指针，表示p指向的内容是常量。需要注意两点：

1. 常量指针指不能通过这个指针改变变量的值，但是还是可以通过其他的引用来改变变量的值的。
2. 常量指针指不能通过这个指针改变变量的值，但是这并不是意味着指针本身不能改变，常量指针可以指向其他的地址。

**第三种情况：**

```c
int a = 9;
int b = 10;
int * const p = &a;

*p = 8;
// p = &b; /* ERROR */
```

指针常量，表示指针p是个常量，它本身不可改变。需要注意的是，指针本身不可变，但指向的对象可变。

**第四种情况：**

```c
int a = 9;
const int * const p = &a;

a = 8;
```

指向常量的指针常量，表示指针本身不可变，也不能通过指针修改所指向地址的内容。需要注意的是，依然可以通过其他引用修改该指针指向地址的内容。

## static

**进程在内存中的布局**

| section | 说明                           |
| ------- | ------------------------------ |
| stack   | 存放局部变量                   |
| head    | 存放由malloc分配               |
| data    | 存放初始化的变量, 静态全局变量 |
| bss     | 未初始化的变量, 静态局部变量   |
| text    | 存放代码                       |

**静态全局变量**

存储在`.data`区，作用是不让其他源文件访问到它自己

**静态局部变量**

存储在`.bss`区的，若未初识化默认为0，每一次访问静态局部变量的值为上一次修改后的值。同样静态局部变量不能被其他函数和源文件访问。

**静态函数**

作用是不让其他源文件访问到它自己，只能被定义该静态函数的源文件访问

## union

共用体的所有成员占用同一段内存，修改一个成员会影响其余所有成员

共用体占用的内存等于最长的成员占用的内存

```bash
## 注意：PC机是小端模式
$ cat test.c
#include <stdio.h>

union data{
    int n;
    char ch;
    short m;
};

int main(){
    union data a;

    printf("%d, %d\n", sizeof(a), sizeof(union data));

    a.n = 0x40;
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);
    a.ch = '9';
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);
    a.m = 0x2059;
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);
    a.n = 0x3E25AD54;
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);

    return 0;
}

$ ./a.out
4, 4
40, @, 40
39, 9, 39
2059, Y, 2059
3E25AD54, T, AD54
```

## 大小端模式

大端模式，是指数据的高字节保存在内存的低地址中，而数据的低字节保存在内存的高地址中

小端模式，是指数据的高字节保存在内存的高地址中，而数据的低字节保存在内存的低地址中

下面以`unsigned int value = 0x12345678`为例，分别看看在两种字节序下其存储情况，我们可以用`unsigned char buf[4]`来表示`value`

| value  | 内存地址 | 小端模式存放内容 | 大端模式存放内容 |
| ------ | -------- | ---------------- | ---------------- |
| buf[0] | 0x4000   | 0x78             | 0x12             |
| buf[1] | 0x4001   | 0x56             | 0x34             |
| buf[2] | 0x4002   | 0x34             | 0x56             |
| buf[3] | 0x4003   | 0x12             | 0x78             |

## 用C语言实现冒泡排序

```c
#include <stdio.h>

#define SIZE 5

int main(int argc, char **argv)
{
	int test[SIZE] = {9, 50, 3, 250, 8};
	int i, j, tmp;

	for(j=1; j<SIZE; j++)
	{
		for(i=0; i<(SIZE-j); i++)
		{
			if(test[i] > test[i+1])
			{
				tmp = test[i];
				test[i] = test[i+1];
				test[i+1] = tmp;
			}
		}
	}

	for(i=0; i<SIZE; i++)
		printf("%d ", test[i]);

	return 0;
}
```

## 不调用库函数，实现strcpy函数的功能

```c
#include <stdio.h>

char *strcpy(char *dest, const char *src)
{
	char *tmp;

	if(dest == NULL || src == NULL)
		return NULL;

	tmp = dest;
	while((*dest++ = *src++) != '\0');

	return tmp;
}

int main(int argc, char **argv)
{
	char *src = "test";
	char dest[1024];

	strcpy(dest, src);
	printf("%s\n", dest);

	return 0;
}
```

## 用C语言实现判断大小端的程序

```c
#include <stdio.h>

union data {
	unsigned int i;
	unsigned char c;
};

int main(int argc, char **argv)
{
	union data test;

	test.i = 0x1;
	if(test.c == 0x01)
		printf("Little-endian\n");
	else
		printf("Big-endian\n");

	return 0;
}
```

## 不调用库函数，实现atoi函数的功能，字符串转整形

```c
#include <stdio.h>
#include <limits.h>

int myatoi(char *s)
{
	char sign = '+';
	int tmp = 0;
	unsigned int value = 0;

	while(*s == ' ')
		*s++;

	if (*s == '+' || *s == '-')
		sign = *s++;


	while (*s >= '0' && *s <= '9') {
		tmp = *s++ - '0';
		value = value * 10 + tmp;

		if (value >= INT_MAX)
			return sign == '+' ? INT_MAX : INT_MIN;
	}

	return sign == '+' ? (int)value : -(int)value;
}

int main(int argc, char *argv[])
{
	printf("str %s, INT_MAX %d, INT_MIN %d\n", argv[1], INT_MAX, INT_MIN);

	printf("int = %d\n", myatoi(argv[1]));

	return 0;
}
```

## 用C语言实现字符串反转功能

```c
#include <stdio.h>
#include <string.h>

void char_swap(char *a, char *b)
{
	char tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void string_reverse(char *s)
{
	int len = strlen(s);
	int i;

	printf("str %s\n", s);

	for (i = 0; i < len/2; i++)
		char_swap(&s[i], &s[len - 1 - i]);

	printf("str %s\n", s);
}


int main(int argc, char *argv[])
{
	char buf[] = "123yabc";

	string_reverse(buf);

	return 0;
}
```

## 获得字符频次唯一的最小删除次数

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LETTER_NUM	26
#define STR_MAX_NUM	100001

int compare_fun(const void *a, const void *b)
{
	return (*(int *)b - *(int *)a);
}

int min_delete(char *s)
{
	int letter[LETTER_NUM] = {0};
	int flags[STR_MAX_NUM] = {0};
	int len = strlen(s);
	int min_delete_bytes = 0;
	int i;

	for (i=0; i<len; i++)
		letter[*s++ - 'a']++;

	qsort(letter, LETTER_NUM, sizeof(int), compare_fun);


	for (i=0; i<LETTER_NUM; i++) {
		if (letter[i] == 0)
			break;

		if (flags[letter[i]] == 0) {
			flags[letter[i]] = 1;
			continue;
		}

		min_delete_bytes++;
		letter[i]--;

		if (letter[i] == 0)
			break;

		if (flags[letter[i]] == 0) {
			flags[letter[i]] = 1;
			continue;
		}
	}

	return min_delete_bytes;
}

int main(int argc, char *argv[])
{
	printf("min_delete_bytes %d\n", min_delete(argv[1]));

	return 0;
}
```

## 亲密字符串

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define LETTER_NUM	26

int compare_fun(const void *a, const void *b)
{
	return (*(int *)b - *(int *)a);
}

bool buddyStrings(char *a, char *b)
{
	int a_len = strlen(a);
	int b_len = strlen(b);
	int i, count = 0, diff1_i, diff2_i;
	int letter_freq[LETTER_NUM] = {0};

	if ((a_len != b_len) || (a_len == 0) || (b_len == 0))
		return false;

	for (i = 0; i < a_len; i++) {
		if (a[i] != b[i]) {
			count++;

			if (diff1_i == 0)
				diff1_i = i;
			else if (diff2_i == 0)
				diff2_i	= i;

			continue;
		}

		letter_freq[a[i] - 'a']++;
	}

	qsort(letter_freq, LETTER_NUM, sizeof(int), compare_fun);

	if (count == 2 && a[diff1_i] == b[diff2_i] && a[diff2_i] == b[diff1_i])
		return true;
	else if (letter_freq[0] >= 2)
		return true;

	return false;
}


int main(int argc, char *argv[])
{
	printf("buddyStrings %d\n", buddyStrings(argv[1], argv[2]));

	return 0;
}
```
