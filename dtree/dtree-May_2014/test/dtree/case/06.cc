#define DTREE_CONFIG EndSeq<WithReverse<WithReverse<WithAggr<dtree::DpMin<double>, dtree::DwLess_<double>, WithAggr<dtree::DpMin<double>, dtree::DwLess_<double>, WithValue<dtree::DpAdd<double>, WithValue<dtree::Nop<int>, WithAggr<dtree::Count<int>, dtree::Index_<int>, Begin > > > > > > > >
#include "test/dtree/seq.cc"
