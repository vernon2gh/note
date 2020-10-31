# 位置参数
def power(x, n):
    s = 1
    while n > 0:
        n = n -1
        s = s * x
    return s

power(5, 2)
power(5, 3)
power(5)

# 默认参数
def power(x, n=2):
    s = 1
    while n > 0:
        n = n -1
        s = s * x
    return s

power(5, 2)
power(5, 3)
power(5)


# 可变参数
#　计算a^2 + b^2 + c^2 + ……
def calc(*numbers):
    sum = 0
    for n in numbers:
        sum = sum + n * n
    return sum

calc()
calc(1)
calc(1, 2)
calc(1, 2, 3)

nums = [1, 2, 3, 4]
calc(*nums)


# 关键字参数
def person(name, age, **kw):
    print('name:', name, ', age:', age, ', other:', kw)

person('vernon', 30)
person('vernon', 30, city='beijing')
person('vernon', 30, city='beijing', job='sofeware')

extra = {'city':'beijing', 'job':'sofeware'}
person('vernon', 30, **extra)



# 命名关键字参数
# 和关键字参数**kw不同，命名关键字参数需要一个特殊分隔符*，*后面的参数被视为命名关键字参数。
def person(name, age, *, city, job):
    print(name, age, city, job)

# 如果函数定义中已经有了一个可变参数，后面跟着的命名关键字参数就不再需要一个特殊分隔符*了：
def person(name, age, *args, city, job):
    print(name, age, args, city, job)

person('vernon', 25, city='beijing', job='software')

extra = {'city':'beijing', 'job':'sofeware'}
extra = {'city':'beijing', 'job':'sofeware', 'addr':'mz'}
person('vernon', 30, **extra)

nums = [1, 2, 3, 4]
person('vernon', 30, *nums, **extra)

# 命名关键字参数可以有缺省值，从而简化调用：
def person(name, age, *, city='beijing', job):
    print(name, age, city, job)

person('vernon', 25, job='software')

# 参数组合
# 在Python中定义函数，可以用必选参数、默认参数、可变参数、关键字参数和命名关键字参数，
# 这5种参数都可以组合使用。但是请注意，参数定义的顺序必须是：
# 必选参数、默认参数、可变参数、命名关键字参数和关键字参数。


# 练习
def product(x, *args):
    sum = 1
    for t in args:
        sum= t * sum
    return x * sum

print('product(5) =', product(5))
print('product(5, 6) =', product(5, 6))
print('product(5, 6, 7) =', product(5, 6, 7))
print('product(5, 6, 7, 9) =', product(5, 6, 7, 9))
if product(5) != 5:
    print('测试失败!')
elif product(5, 6) != 30:
    print('测试失败!')
elif product(5, 6, 7) != 210:
    print('测试失败!')
elif product(5, 6, 7, 9) != 1890:
    print('测试失败!')
else:
    try:
        product()
        print('测试失败!')
    except TypeError:
        print('测试成功!')











# 小结

# Python的函数具有非常灵活的参数形态，既可以实现简单的调用，又可以传入非常复杂的参数。

# 默认参数一定要用不可变对象，如果是可变对象，程序运行时会有逻辑错误！

# 要注意定义可变参数和关键字参数的语法：

# *args是可变参数，args接收的是一个tuple；

# **kw是关键字参数，kw接收的是一个dict。

# 以及调用函数时如何传入可变参数和关键字参数的语法：

# 可变参数既可以直接传入：func(1, 2, 3)，又可以先组装list或tuple，再通过*args传入：func(*(1, 2, 3))；

# 关键字参数既可以直接传入：func(a=1, b=2)，又可以先组装dict，再通过**kw传入：func(**{'a': 1, 'b': 2})。

# 使用*args和**kw是Python的习惯写法，当然也可以用其他参数名，但最好使用习惯用法。

# 命名的关键字参数是为了限制调用者可以传入的参数名，同时可以提供默认值。

# 定义命名的关键字参数在没有可变参数的情况下不要忘了写分隔符*，否则定义的将是位置参数。


