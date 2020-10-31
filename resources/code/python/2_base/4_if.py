age = 2
if age >= 18:
    print('a')
elif age >= 6:
    print('666')
else:
    print('fail')



# birth = input('birth: ')
birth = int(input('birth: '))

if birth < 2000:
    print('000 before')
else:
    print('000 after')


height = 1.75
weight = 80.5
bmi = weight / (height*height)
if bmi < 18.5:
    print('过轻')
elif bmi < 25:
    print('正常')
elif bmi < 28:
    print('过重')
elif bmi < 32:
    print('肥胖')
else:
    print('严重肥胖')


    
