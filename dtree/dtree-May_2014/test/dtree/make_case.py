#! /usr/bin/env python3
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

from random import seed
import seq
import tree

seed(1)
template = '''#define DTREE_CONFIG {config}
#include "test/dtree/{name}.cc"
'''
tests = []
for count, mod in [(40, seq), (60, tree)]:
    tests.extend(template.format(
        config=mod.randconfig(), name=mod.__name__) for i in range(count))
for i, test in enumerate(tests):
    with open('case/{:02}.cc'.format(i), 'w') as f:
        f.write(test)
