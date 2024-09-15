#define DTREE_CONFIG EndSeq<WithReverse<WithReverse<WithAggr<dtree::CountAndSum<double, double>, dtree::IndexByCount_<double>, WithAggr<dtree::Max<double>, dtree::GreaterEqual_<double>, WithValue<dtree::Add<double>, Begin > > > > > >
#include "test/dtree/seq.cc"
