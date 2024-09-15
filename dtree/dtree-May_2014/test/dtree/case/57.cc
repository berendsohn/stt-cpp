#define DTREE_CONFIG EndTree<WithAncAggr<dtree::Sum<int>, dtree::Index_<int>, WithDescValue<dtree::Nop<int>, WithDescValue<FreeGroup, WithAncAggr<dtree::Count<int>, dtree::Index_<int>, WithAncAggr<dtree::Count<int>, dtree::Index_<int>, Begin > > > > > >
#include "test/dtree/tree.cc"
