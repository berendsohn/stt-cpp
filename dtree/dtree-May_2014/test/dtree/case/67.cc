#define DTREE_CONFIG EndTree<WithDescValue<dtree::Nop<int>, WithAncAggr<dtree::Max<int>, dtree::GreaterEqual_<int>, WithAncAggr<dtree::Min<int>, dtree::Less_<int>, WithDescValue<dtree::Nop<int>, Begin > > > > >
#include "test/dtree/tree.cc"
