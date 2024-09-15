#define DTREE_CONFIG EndSeq<WithReverse<WithAggr<dtree::Sum<int>, dtree::Index_<int>, WithValue<dtree::Nop<int>, Begin > > > >
#include "test/dtree/seq.cc"
