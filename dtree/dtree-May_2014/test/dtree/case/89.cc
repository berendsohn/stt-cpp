#define DTREE_CONFIG EndTreeWithDesc<WithDescAggrOfDescValue<dtree::DpMin<double>, dtree::DwLessEqual_<double>, WithStaticValue<dtree::DpValue<double>, WithAncDescAggr<dtree::Count<int>, NullPredicate_, Begin > > > >
#include "test/dtree/tree.cc"
