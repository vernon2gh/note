############## dict ##################
d = {'a' : 95, 'b' : 90, 'cc' : 70}
d['a'] = 66
d['a'] = 655
d['aa'] = 44


d['a']
d['aa']


d['aab']
'aab' in d

d.get('aa')
d.get('aab')
d.get('aab', -1)

d.pop('aa')
d

key = [1, 2, 3]
d[key] = 55


############## set #################
s = set([1, 2, 3])
s = set([1, 1, 2, 2, 2, 3, 3])
s.add(4)
s.remove(4)

s

s1 = set([1, 2, 3])
s2 = set([2, 3, 4])
s1 & s2
s1 | s2

key = [1, 6, 3]
s.add(key)



#####################################




