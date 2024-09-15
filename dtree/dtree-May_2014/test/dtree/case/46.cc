#define DTREE_CONFIG EndTreeWithDesc<WithEvert<WithEvert<WithAncAggr<dtree::DpMin<double>, dtree::DwLessEqual_<double>, WithAncAggr<dtree::DpMin<double>, dtree::DwLessEqual_<double>, WithAncValue<dtree::DpAdd<double>, WithDescAggrOfDescValue<dtree::Count<int>, NullPredicate_, WithAncAggr<dtree::Count<int>, dtree::Index_<int>, Begin > > > > > > > >
#include "test/dtree/tree.cc"
