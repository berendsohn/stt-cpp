#define DTREE_CONFIG EndSeq<WithReverse<WithReverse<WithAggr<dtree::CountAndSum<double, double>, dtree::IndexByCount_<double>, WithValue<dtree::Add<double>, WithAggr<dtree::Count<int>, dtree::Index_<int>, WithAggr<dtree::Count<int>, dtree::Index_<int>, Begin > > > > > > >
#include "test/dtree/seq.cc"
