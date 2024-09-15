// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_SEQ_H_
#define DTREE_SEQ_H_

#include "dtree/selector.h"

#ifdef NAIVE_DTREE
#include "dtree/naive/seq.h"
#else
#include "dtree/selfadjust/seq.h"
#endif
#endif  // DTREE_SEQ_H_
