# list和tuple是Python内置的有序集合，一个可变，一个不可变。根据需要来选择使用它们

classmates = ['a', 'b', 'cc']
classmates

len(classmates)

classmates[0]
classmates[1]
classmates[2]
classmates[3]


classmates[-1]
classmates[-2]
classmates[-3]
classmates[-4]

classmates.append('ddd')
classmates

classmates.insert(1, 'yyy')
classmates

classmates.pop()
classmates

classmates.pop(1)
classmates

classmates[0] = 'fff'
classmates

l = ['aaa', 11, True, ['b', 'c']]
l

len(l)

cc = [1, 2, 3]
a = ['a', 'b', cc]
a
cc[1]
a[2][1]

b = []
len(b)

#######################################

classmates = ('a', 'b', 'cc')
classmates

classmates[0]
classmates[-1]
classmates[0] = 'bb'


L = [
    ['Apple', 'Google', 'Microsoft'],
    ['Java', 'Python', 'Ruby', 'PHP'],
    ['Adam', 'Bart', 'Lisa']
]

print(L[0][0])
print(L[1][1])
print(L[2][2])


