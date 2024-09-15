#define DTREE_CONFIG EndTreeWithDesc<WithAncAggr<dtree::Max<int>, dtree::Greater_<int>, WithDescValueWithDesc<dtree::Nop<int>, WithStaticValue<int, WithDescAggrOfDescValue<dtree::Count<int>, NullPredicate_, Begin > > > > >
#include "test/dtree/tree.cc"
