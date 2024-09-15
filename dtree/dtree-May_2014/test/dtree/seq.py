# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

from common import *


def randaggr(typstr, ops, node):
    for j in range(randrange(3)):
        op = choice(ops)
        opstr = op.format.format(typ=typstr)
        pred = choice(op.predicates)
        predstr = pred.format.format(typ=typstr)
        fmt = 'WithAggr<{op}, {pred}, {node} >'
        node = fmt.format(op=opstr, pred=predstr, node=node)
    return node


def randconfig():
    node = 'Begin'
    node = randaggr('int', [s_Count], node)
    for j in range(randrange(3)):
        typ = choice(typs)
        typstr = typ.format
        grp = choice(typ.groups)
        grpstr = grp.format.format(typ=typstr)
        fmt = 'WithValue<{grp}, {node} >'
        node = fmt.format(grp=grpstr, node=node)
        node = randaggr(typstr, grp.operations, node)
        if grp is g_DpAdd and randrange(2):
            node = 'WithReverseBy<{node} >'.format(node=node)
    for j in range(randrange(3)):
        node = 'WithReverse<{node} >'.format(node=node)
    return 'EndSeq<{node} >'.format(node=node)
