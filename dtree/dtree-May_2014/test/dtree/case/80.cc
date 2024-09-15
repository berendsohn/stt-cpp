#define DTREE_CONFIG EndTreeWithDesc<WithAncDescAggr<dtree::DpMax<double>, dtree::DwGreaterEqual_<double>, WithAncAggr<dtree::DpMin<double>, dtree::DwLess_<double>, WithAncValue<dtree::DpAdd<double>, WithAncDescAggr<dtree::Count<int>, NullPredicate_, Begin > > > > >
#include "test/dtree/tree.cc"
