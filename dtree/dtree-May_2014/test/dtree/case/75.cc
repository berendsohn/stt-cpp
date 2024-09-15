#define DTREE_CONFIG EndTree<WithAncAggr<dtree::CountAndSum<double, double>, dtree::IndexByCount_<double>, WithAncAggr<dtree::Max<double>, dtree::Greater_<double>, WithAncValue<dtree::Add<double>, WithDescValue<dtree::Nop<int>, Begin > > > > >
#include "test/dtree/tree.cc"
