// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_SELECTOR_H_
#define DTREE_SELECTOR_H_

#ifdef NAIVE_DTREE
namespace naive {
}  // namespace naive
namespace dtree {
using namespace naive;
}  // namespace dtree
#else
namespace selfadjust {
}  // namespace selfadjust
namespace dtree {
using namespace selfadjust;
}  // namespace dtree
#endif
#endif  // DTREE_SELECTOR_H_
