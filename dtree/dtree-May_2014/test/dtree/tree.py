# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

from common import *


def randaggr(vscope, typstr, ascopes, ops, node):
    for j in range(randrange(3)):
        ascope = choice(ascopes)
        op = choice(ops)
        opstr = op.format.format(typ=typstr)
        pred = choice(op.predicates)
        if ascope == 'Desc' and pred in {p_Index, p_IndexByCount}:
            pred = p_NullPredicate
        predstr = pred.format.format(typ=typstr)
        if ascope == 'Anc':
            cls = 'AncAggr'
        elif ascope == 'Desc' and vscope in {'Anc', 'AncDesc'}:
            cls = 'AncDescAggr'
        elif ascope == 'Desc' and vscope in {'Static', 'Desc'}:
            cls = choice(['DescAggrOfDescValue', 'AncDescAggr'])
        else:
            assert False
        node = 'With{cls}<{op}, {pred}, {node} >'.format(
            cls=cls, op=opstr, pred=predstr, node=node)
    return node


def randconfig():
    end, ascopes = choice(
        [('EndTree', ['Anc']),
         ('EndTreeWithDesc', ['Anc', 'Desc', 'Desc'])])
    node = 'Begin'
    node = randaggr('Desc', 'int', ascopes, [s_Count], node)
    for j in range(randrange(3)):
        typ = choice(typs)
        typstr = typ.format
        grp = choice(typ.groups)
        grpstr = grp.format.format(typ=typstr)
        vscope = choice(['Static', 'Anc', 'Desc', 'AncDesc'])
        if vscope == 'AncDesc' and grp is not g_FreeGroup:
            vscope = choice(['Anc', 'Desc'])
        if end == 'EndTreeWithDesc' and vscope in {'Desc', 'AncDesc'}:
            fmt = 'With{vscope}ValueWithDesc'
        else:
            fmt = 'With{vscope}Value'
        cls = fmt.format(vscope=vscope)
        if vscope == 'Static':
            grpstr = ('dtree::DpValue<{typ}>'.format(typ=typstr)
                      if grp is g_DpAdd
                      else typstr)
        node = '{cls}<{grp}, {node} >'.format(
            cls=cls, grp=grpstr, node=node)
        node = randaggr(vscope, typstr, ascopes, grp.operations, node)
    for j in range(randrange(3)):
        node = 'WithEvert<{node} >'.format(node=node)
    return '{end}<{node} >'.format(end=end, node=node)
