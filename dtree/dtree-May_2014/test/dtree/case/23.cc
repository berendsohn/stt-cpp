#define DTREE_CONFIG EndSeq<WithReverse<WithReverse<WithAggr<dtree::Or<int>, dtree::NonzeroAnd_<int>, WithAggr<dtree::Min<int>, dtree::LessEqual_<int>, WithValue<dtree::Nop<int>, WithAggr<dtree::Count<int>, dtree::Index_<int>, Begin > > > > > > >
#include "test/dtree/seq.cc"
