// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_NAIVE_SEQ_INL_H_
#define DTREE_NAIVE_SEQ_INL_H_

namespace naive {

static const int kLeft = dtree::kLw;
static const int kRight = dtree::kRw;

template<typename Base>
template<typename Forwarder>
inline void EndSeq<Base>::CopyFrom(const EndSeq& node, Forwarder forward) {
  Base::operator=(node);
  set_solid(0, node.solid(0) ? forward(this, &node, node.solid(0)) : NULL);
  set_solid(1, node.solid(1) ? forward(this, &node, node.solid(1)) : NULL);
}

template<typename Node>
Node* Dir(int dir, Node* node) {
  if (!node) return NULL;
  while (Node* solid_dir = node->solid(dir)) node = solid_dir;
  return node;
}

template<typename Node>
inline Node* Left(Node* node) {
  return Dir(kLeft, node);
}

template<typename Node>
inline Node* Right(Node* node) {
  return Dir(kRight, node);
}

template<typename Node>
inline bool SameSeq(Node* node1, Node* node2) {
  return Right(node1) == Right(node2);
}

template<typename Node>
inline Node* CutDirOf(int dir, Node* node2) {
  if (!node2) return NULL;
  Node* node1 = node2->solid(dir);
  if (!node1) return NULL;
  node2->set_solid(dir, NULL);
  node1->set_solid(dir ^ 1, NULL);
  return node1;
}

template<typename Node>
inline Node* CutLeftOf(Node* node2) {
  return CutDirOf(kLeft, node2);
}

template<typename Node>
inline Node* CutRightOf(Node* node2) {
  return CutDirOf(kRight, node2);
}

template<typename Node>
inline Node* LinkDirOf(int dir, Node* node1, Node* node2) {
  if (!node1) return node2;
  if (!node2) return node1;
  node1 = Dir(dir ^ 1, node1);
  node2 = Dir(dir, node2);
  node2->set_solid(dir, node1);
  node1->set_solid(dir ^ 1, node2);
  return node2;
}

template<typename Node>
inline Node* LinkLeftOf(Node* node1, Node* node2) {
  return LinkDirOf(kLeft, node1, node2);
}

template<typename Node>
inline Node* LinkRightOf(Node* node1, Node* node2) {
  return LinkDirOf(kRight, node1, node2);
}

template<typename Node>
class ScopedCutDirOf {
 public:
  ScopedCutDirOf(int dir, Node* node2)
      : dir_(dir), excl_part_(CutDirOf(dir, node2)), incl_part_(node2) {
  }
  ~ScopedCutDirOf() { LinkDirOf(dir(), excl_part(), incl_part()); }
  int dir() const { return dir_; }
  Node* excl_part() const { return excl_part_; }
  Node* incl_part() const { return incl_part_; }
 private:
  int dir_;
  Node* excl_part_;
  Node* incl_part_;
  DISALLOW_COPY_AND_ASSIGN(ScopedCutDirOf);
};

template<typename Node>
class ScopedCutLeftOf : public ScopedCutDirOf<Node> {
 public:
  explicit ScopedCutLeftOf(Node* node2) : ScopedCutDirOf<Node>(kLeft, node2) {}
 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedCutLeftOf);
};

template<typename Node>
class ScopedCutRightOf : public ScopedCutDirOf<Node> {
 public:
  explicit ScopedCutRightOf(Node* node2)
      : ScopedCutDirOf<Node>(kRight, node2) {
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedCutRightOf);
};

template<typename Node>
inline Node* Dirward(int dir, Node* node) {
  return node ? node->solid(dir) : NULL;
}

template<typename Node>
inline Node* Leftward(Node* node) {
  return Dirward(kLeft, node);
}

template<typename Node>
inline Node* Rightward(Node* node) {
  return Dirward(kRight, node);
}

template<typename Group, typename Base>
template<typename Node>
inline typename Group::Type WithValue<Group, Base>::Value(Node* node) {
  typedef WithValue Self;
  return node->Self::value();
}

template<typename Group, typename Base>
template<typename Node>
inline void WithValue<Group, Base>::SetValue(Node* node,
                                             typename Group::Type value) {
  typedef WithValue Self;
  node->Self::set_value(value);
}

template<typename Group, typename Base>
template<typename Node>
void WithValue<Group, Base>::AddToSeq(Node* node, typename Group::Type delta) {
  typedef WithValue Self;
  for (node = Left(node); node; node = node->solid(kRight)) {
    node->Self::add_to_value(delta);
  }
}

template<typename Group, typename Base>
template<typename Node>
void WithValue<Group, Base>::SubtractFromSeq(Node* node,
                                             typename Group::Type delta) {
  typedef WithValue Self;
  for (node = Left(node); node; node = node->solid(kRight)) {
    node->Self::subtract_from_value(delta);
  }
}

template<typename Semigroup, typename Base>
template<typename Node>
typename Semigroup::Type WithAggr<Semigroup, Base>::AggrSeq(Node* node) {
  typedef WithAggr Self;
  typename Semigroup::Type aggr = Semigroup::empty_aggr();
  for (node = Left(node); node; node = node->solid(kRight)) {
    aggr = Semigroup::CombineAggrs(aggr, node->Self::singleton_aggr());
  }
  return aggr;
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
Node* WithAggr<Semigroup, Base>::FindDirmostSeq(int dir,
                                                Node* node,
                                                const Predicate& predicate) {
  typedef WithAggr Self;
  typename Semigroup::Type cum_aggr = Semigroup::empty_aggr();
  for (node = Dir(dir, node); node; node = node->solid(dir ^ 1)) {
    cum_aggr = Semigroup::CombineAggrs(cum_aggr, node->Self::singleton_aggr());
    if (predicate(cum_aggr)) return node;
  }
  return NULL;
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
inline Node* WithAggr<Semigroup, Base>::FindLeftmostSeq(
    Node* node,
    const Predicate& predicate) {
  return FindDirmostSeq(kLeft, node, predicate);
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
inline Node* WithAggr<Semigroup, Base>::FindRightmostSeq(
    Node* node,
    const Predicate& predicate) {
  return FindDirmostSeq(kRight, node, predicate);
}

template<typename Base>
template<typename Node>
void WithReverseBy<Base>::ReverseSeq(Node* node) {
  typedef WithReverseBy Self;
  for (node = Left(node); node; node = node->solid(kLeft)) {
    node->Self::add_to_value(Group::flip_delta());
    Node* temp = node->solid(kRight);
    node->set_solid(kRight, node->solid(kLeft));
    node->set_solid(kLeft, temp);
  }
}

template<typename Base>
template<typename Node>
void WithReverse<Base>::ReverseSeq(Node* node) {
  for (node = Left(node); node; node = node->solid(kLeft)) {
    Node* temp = node->solid(kRight);
    node->set_solid(kRight, node->solid(kLeft));
    node->set_solid(kLeft, temp);
  }
}
}  // namespace naive
#endif  // DTREE_NAIVE_SEQ_INL_H_
