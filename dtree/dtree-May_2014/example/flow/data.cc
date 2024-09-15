// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include "example/flow/data.h"

#include <assert.h>
#include <limits>
#include <set>
#include <utility>

namespace flow {

ArcDescriptor::ArcDescriptor() : tail(0), head(0), cap(0) {
}

ArcDescriptor::ArcDescriptor(int tail, int head, int cap)
    : tail(tail), head(head), cap(cap) {
}

void ArcDescriptor::Validate(int n) const {
  assert(tail >= 1);
  assert(tail <= n);
  assert(head >= 1);
  assert(head <= n);
  assert(tail != head);
  assert(cap >= 0);
}

void ArcDescriptor::Reset() {
  tail = 0;
  head = 0;
  cap = 0;
}

Network::Network() : n(0), s(0), t(0), arcs() {
}

void Network::Validate() const {
  assert(n >= 2);
  assert(n <= std::numeric_limits<int>::max() - 1);
  assert(arcs.size() <=
         static_cast<std::vector<ArcDescriptor>::size_type>(
             std::numeric_limits<int>::max()));
  assert(s >= 1);
  assert(s <= n);
  assert(t >= 1);
  assert(t <= n);
  assert(s != t);
  std::set<std::pair<int, int> > arc_set;
  for (std::vector<ArcDescriptor>::const_iterator i = arcs.begin();
       i != arcs.end();
       ++i) {
    i->Validate(n);
    bool not_parallel =
        arc_set.insert(std::make_pair(i->tail, i->head)).second;
    assert(not_parallel);
  }
}

void Network::Reset() {
  n = 0;
  s = 0;
  t = 0;
  arcs.clear();
}

Solution::Solution() : value(0), flows() {
}

void Solution::InitializeFrom(const Network& network) {
  Reset();
  flows.resize(network.arcs.size(), 0);
}

void Solution::Validate(const Network& network) const {
  assert(value >= 0);
  assert(network.arcs.size() == flows.size());
  std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
  for (std::vector<int>::const_iterator j = flows.begin();
       j != flows.end();
       ++i, ++j) {
    assert(*j >= 0);
    assert(*j <= i->cap);
  }
}

void Solution::Reset() {
  value = 0;
  flows.clear();
}
}  // namespace flow
