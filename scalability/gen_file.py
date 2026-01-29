from random import shuffle

# Simply script for generating valid .in files for different sizes

INPUT_SIZE = 512

with open("%s.in" % INPUT_SIZE, 'w') as f:
	f.write(str(INPUT_SIZE) + '\n')
	for i in range(INPUT_SIZE * 2):
		l = [str(j) for j in range(1, INPUT_SIZE + 1)]
		shuffle(l)
		f.write(' '.join(l) + '\n')