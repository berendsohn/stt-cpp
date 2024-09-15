#define DTREE_CONFIG EndSeq<WithReverse<WithReverse<WithAggr<dtree::Sum<int>, dtree::Index_<int>, WithAggr<dtree::Max<int>, dtree::GreaterEqual_<int>, WithValue<dtree::Nop<int>, WithAggr<dtree::Min<int>, dtree::Less_<int>, WithValue<dtree::Nop<int>, Begin > > > > > > > >
#include "test/dtree/seq.cc"
