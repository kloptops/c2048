#/usr/local/bin python

import sys

if len(sys.argv) < 2:
    print "Usage: {0} <number>".format(sys.argv[0])

size = int(sys.argv[1])

output = list(range(1, (size * size) + 1));
for i in range(1, size, 2):
    output[(i*size):((i+1) * size)] = output[(i*size):((i+1) * size)][::-1]

output[:] = output[::-1]

print "static const uint32_t _rowscore_weight[{}] = {{".format(size * size)
for i in range(size):
    print ("   " + " {:2d}," * size).format(*output[(i*size):((i+1) * size)])
print "    };"
