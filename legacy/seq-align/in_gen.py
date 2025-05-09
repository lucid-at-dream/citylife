


alphabet = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPKRSTUVWXYZ'

from random import choice
from sys import argv

n = int(argv[1])
m = int(argv[2])

a = "".join([choice(alphabet) for i in range(n)])
b = "".join([choice(alphabet) for i in range(m)])

print(n,m)
print(a)
print(b)

