// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_NAIVE_COMMON_H_
#define DTREE_NAIVE_COMMON_H_

#include <stddef.h>

#include "dtree/type.h"
#include "util/disallow_copy_and_assign.h"

namespace naive {

class Empty {
 protected:
  Empty() {}
};

template<typename Base = Empty>
class Begin : public Base {
 public:
  typedef dtree::Nop<dtree::NoValue> Group_;
  dtree::NoValue value() const { return dtree::NoValue(); }
 protected:
  Begin() {}
};
}  // namespace naive
#endif  // DTREE_NAIVE_COMMON_H_
