#define DTREE_CONFIG EndSeq<WithReverse<WithAggr<dtree::DpMax<double>, dtree::DwGreaterEqual_<double>, WithValue<dtree::DpAdd<double>, Begin > > > >
#include "test/dtree/seq.cc"
