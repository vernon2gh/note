############ for x in ... ###############
names = ['a', 'b', 'cc']
for name in names:
    print(name)

list(range(5))

sum = 0
for i in range(101):
    sum = sum + i
print(sum)


############ while ###############
sum = 0
n = 99
while n > 0:
    sum = sum + n
    n = n -2
print(sum)

i = 0
L = ['Bart', 'Lisa', 'Adam']
while i < 3:
    print(L[i])
    i = i + 1

################ break #############
n = 1
while n <= 100:
    if n > 10:
        break

    print(n)
    n = n + 1
print('END')


################ continue #############
n = 0
while n < 10:
    n = n + 1
    if n % 2 == 0:
        continue
    print(n)




