from random import randrange


filename = 'test_arrays2.txt'
arrnum = 50
f = open(filename, mode='w')
for i in range(arrnum):
    size = randrange(2, 41)
    for j in range(size):
        f.write(str(randrange(0, 1000)))
        f.write(' ')
    f.write('\n')
f.close()
