#define DTREE_CONFIG EndTree<WithEvert<WithEvert<WithAncAggr<dtree::Min<int>, dtree::Less_<int>, WithAncAggr<dtree::Min<int>, dtree::LessEqual_<int>, WithDescValue<dtree::Nop<int>, WithAncAggr<dtree::DpMax<double>, dtree::DwGreaterEqual_<double>, WithDescValue<dtree::DpAdd<double>, Begin > > > > > > > >
#include "test/dtree/tree.cc"
