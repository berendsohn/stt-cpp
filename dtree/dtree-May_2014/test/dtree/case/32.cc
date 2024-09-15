#define DTREE_CONFIG EndSeq<WithReverse<WithAggr<dtree::CountAndSum<double, double>, dtree::IndexByCount_<double>, WithAggr<dtree::Min<double>, dtree::Less_<double>, WithValue<dtree::Add<double>, WithValue<dtree::Add<double>, Begin > > > > > >
#include "test/dtree/seq.cc"
