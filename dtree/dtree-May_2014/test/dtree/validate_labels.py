#! /usr/bin/env python3
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

import fileinput
import re

pat1 = re.compile('kNumOutcomes = (0|[1-9][0-9]*),$')
pat2 = re.compile('case (0|[1-9][0-9]*):$')
n = 0
for line in fileinput.input():
    line = line.strip()
    m1 = pat1.match(line)
    if m1:
        print(line)
        count = m1.group(1)
        continue
    m2 = pat2.match(line)
    if m2:
        print(line)
        assert m2.group(1) == str(n)
        n += 1
        continue
    if line == 'default:':
        print(line)
        assert count == str(n)
        n = 0
        continue
