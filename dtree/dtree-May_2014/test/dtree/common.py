# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

from collections import namedtuple
from random import choice, randrange

Predicate = namedtuple('Predicate', ['format'])
p_Less = Predicate('dtree::Less_<{typ}>')
p_LessEqual = Predicate('dtree::LessEqual_<{typ}>')
p_Greater = Predicate('dtree::Greater_<{typ}>')
p_GreaterEqual = Predicate('dtree::GreaterEqual_<{typ}>')
p_Index = Predicate('dtree::Index_<{typ}>')
p_IndexByCount = Predicate('dtree::IndexByCount_<{typ}>')
p_Nonzero = Predicate('dtree::Nonzero_')
p_NonzeroAnd = Predicate('dtree::NonzeroAnd_<{typ}>')
p_DwLess = Predicate('dtree::DwLess_<{typ}>')
p_DwLessEqual = Predicate('dtree::DwLessEqual_<{typ}>')
p_DwGreater = Predicate('dtree::DwGreater_<{typ}>')
p_DwGreaterEqual = Predicate('dtree::DwGreaterEqual_<{typ}>')
p_NullPredicate = Predicate('NullPredicate_')

Semigroup = namedtuple('Semigroup', ['format', 'predicates'])
s_Min = Semigroup('dtree::Min<{typ}>', [p_Less, p_LessEqual])
s_Max = Semigroup('dtree::Max<{typ}>', [p_Greater, p_GreaterEqual])
s_Count = Semigroup('dtree::Count<{typ}>', [p_Index])
s_Sum = Semigroup('dtree::Sum<{typ}>', [p_Index])
s_CountAndSum = Semigroup('dtree::CountAndSum<{typ}, {typ}>', [p_IndexByCount])
s_Or = Semigroup('dtree::Or<{typ}>', [p_Nonzero, p_NonzeroAnd])
s_DpMin = Semigroup('dtree::DpMin<{typ}>', [p_DwLess, p_DwLessEqual])
s_DpMax = Semigroup('dtree::DpMax<{typ}>', [p_DwGreater, p_DwGreaterEqual])
s_FreeSemigroup = Semigroup('FreeSemigroup', [p_NullPredicate])

Group = namedtuple('Group', ['format', 'operations'])
g_Nop = Group('dtree::Nop<{typ}>', [s_Min, s_Max, s_Sum, s_CountAndSum, s_Or])
g_Add = Group('dtree::Add<{typ}>', [s_Min, s_Max, s_CountAndSum])
g_DpAdd = Group('dtree::DpAdd<{typ}>', [s_DpMin, s_DpMax])
g_FreeGroup = Group('FreeGroup', [s_FreeSemigroup])

Type = namedtuple('Type', ['format', 'groups'])
t_int = Type('int', [g_Nop])
t_double = Type('double', [g_Add, g_DpAdd])
t_free = Type('FreeValue', [g_FreeGroup])
typs = [t_int, t_double, t_free]
