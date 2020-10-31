def my_abs(x):
    if x >= 0:
        return x
    else:
        return -x
my_abs(-222)
my_abs('aa')
abs('aa')


def nop():
   pass
nop()



def my_abs(x):
    if not isinstance(x, (int, float)):
        raise TypeError('bad operand type')

    if x >= 0:
        return x
    else:
        return -x
my_abs('aa')
abs('aa')


import math
def move(x, y, step, angle=0):
    nx = x + step * math.cos(angle)
    ny = y - step * math.sin(angle)
    return nx, ny

x, y = move(100, 100, 60, math.pi / 6)
print(x, y)

r = move(100, 100, 60, math.pi / 6)
print(r)



####################练习#########################
import math
def quadratic(a, b, c):
    a1 = (-b + math.sqrt(b*b - 4*a*c)) / (2*a)
    a2 = (-b - math.sqrt(b*b - 4*a*c)) / (2*a)
    return a1, a2

quadratic(2, 3, 1)

print('quadratic(2, 3, 1) =', quadratic(2, 3, 1))
print('quadratic(1, 3, -4) =', quadratic(1, 3, -4))

if quadratic(2, 3, 1) != (-0.5, -1.0):
    print('测试失败')
elif quadratic(1, 3, -4) != (1.0, -4.0):
    print('测试失败')
else:
    print('测试成功')

#小结
#定义函数时，需要确定函数名和参数个数；
#如果有必要，可以先对参数的数据类型做检查；
#函数体内部可以用return随时返回函数结果；
#函数执行完毕也没有return语句时，自动return None。
#函数可以同时返回多个值，但其实就是一个tuple。
