#define DTREE_CONFIG EndSeq<WithReverse<WithAggr<dtree::DpMax<double>, dtree::DwGreater_<double>, WithValue<dtree::DpAdd<double>, WithValue<dtree::Nop<int>, Begin > > > > >
#include "test/dtree/seq.cc"
