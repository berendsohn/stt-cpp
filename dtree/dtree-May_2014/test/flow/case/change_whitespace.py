#! /usr/bin/env python3
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

import glob
import random
import re


def randspace(a, b):
    return ''.join(random.choice('\t ') for i in range(a, b))


def changewhite(line):
    chunks = []
    for i, word in enumerate(line.split()):
        a = 1 if i > 0 else 0
        chunks.append(''.join(random.choice('\t ') for i in range(a, a + 2)))
        chunks.append(word)
    chunks.append(randspace(0, 2))
    return ''.join(chunks)


random.seed(1)
paths = glob.glob('*.txt')
paths.sort(key=lambda path: int(path[:-4]))
for path in paths:
    terminators = ('\r', '\r\n', '\n')
    with open(path) as f:
        lines = re.split('|'.join(terminators), f.read())
    terminator = random.choice(terminators)
    with open(path, 'w') as f:
        f.write(terminator.join(map(changewhite, lines)))
