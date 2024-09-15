// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_TREE_H_
#define DTREE_TREE_H_

#include "dtree/selector.h"

#ifdef NAIVE_DTREE
#include "dtree/naive/tree.h"
#else
#include "dtree/selfadjust/tree.h"
#endif
#endif  // DTREE_TREE_H_
