#define DTREE_CONFIG EndTree<WithEvert<WithAncAggr<dtree::DpMin<double>, dtree::DwLessEqual_<double>, WithDescValue<dtree::DpAdd<double>, WithAncAggr<dtree::Count<int>, dtree::Index_<int>, Begin > > > > >
#include "test/dtree/tree.cc"
