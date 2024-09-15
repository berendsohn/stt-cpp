// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_NAIVE_TREE_INL_H_
#define DTREE_NAIVE_TREE_INL_H_

namespace naive {

static const int kLeaf = dtree::kLw;
static const int kRoot = dtree::kRw;

template<typename Base>
template<typename Forwarder>
inline void EndTree<Base>::CopyFrom(const EndTree& node, Forwarder forward) {
  Base::operator=(node);
  set_parent(node.parent() ? forward(this, &node, node.parent()) : NULL);
}

template<typename Node>
Node* Root(Node* node) {
  if (!node) return NULL;
  while (Node* p = node->parent()) node = p;
  return node;
}

template<typename Node>
inline bool SameTree(Node* node1, Node* node2) {
  return Root(node1) == Root(node2);
}

template<typename Node>
Node* LeafmostCommonAnc(Node* node1, Node* node2) {
  if (!(node1 && node2)) return NULL;
  Node* root1 = node1;
  int depth1 = 0;
  while (Node* p = root1->parent()) {
    root1 = p;
    ++depth1;
  }
  Node* root2 = node2;
  int depth2 = 0;
  while (Node* p = root2->parent()) {
    root2 = p;
    ++depth2;
  }
  if (root1 != root2) return NULL;
  if (depth1 > depth2) {
    do {
      node1 = node1->parent();
      --depth1;
    } while (depth1 > depth2);
  } else {
    while (depth2 > depth1) {
      node2 = node2->parent();
      --depth2;
    }
  }
  while (node1 != node2) {
    node1 = node1->parent();
    node2 = node2->parent();
  }
  return node2;
}

template<typename Node>
inline Node* Cut(Node* node2) {
  if (!node2) return NULL;
  Node* node1 = node2->parent();
  node2->set_parent(NULL);
  return node1;
}

template<typename Node>
inline Node* Link(Node* node1, Node* node2) {
  if (!node2) return node1;
  if (node1) Root(node1)->set_parent(node2);
  return node2;
}

template<typename Node>
inline Node* Parent(Node* node) {
  return node ? node->parent() : NULL;
}

template<typename Type, typename Base>
template<typename Node>
inline Type WithStaticValue<Type, Base>::Value(Node* node) {
  typedef WithStaticValue Self;
  return node->Self::value();
}

template<typename Type, typename Base>
template<typename Node>
inline void WithStaticValue<Type, Base>::SetValue(Node* node, Type value) {
  typedef WithStaticValue Self;
  node->Self::set_value(value);
}

template<typename Group, typename Base>
template<typename Node>
inline typename Group::Type WithAncValue<Group, Base>::Value(Node* node) {
  typedef WithAncValue Self;
  return node->Self::value();
}

template<typename Group, typename Base>
template<typename Node>
inline void WithAncValue<Group, Base>::SetValue(Node* node,
                                                typename Group::Type value) {
  typedef WithAncValue Self;
  node->Self::set_value(value);
}

template<typename Group, typename Base>
template<typename Node>
void WithAncValue<Group, Base>::AddToAnc(Node* node,
                                         typename Group::Type delta) {
  typedef WithAncValue Self;
  for (; node; node = node->parent()) node->Self::add_to_value(delta);
}

template<typename Group, typename Base>
template<typename Node>
void WithAncValue<Group, Base>::SubtractFromAnc(Node* node,
                                                typename Group::Type delta) {
  typedef WithAncValue Self;
  for (; node; node = node->parent()) node->Self::subtract_from_value(delta);
}

template<typename Group, typename Base>
template<typename Node>
inline void WithAncValue<Group, Base>::AddToProperAnc(
    Node* node,
    typename Group::Type delta) {
  AddToAnc(Parent(node), delta);
}

template<typename Group, typename Base>
template<typename Node>
inline void WithAncValue<Group, Base>::SubtractFromProperAnc(
    Node* node,
    typename Group::Type delta) {
  SubtractFromAnc(Parent(node), delta);
}

template<typename Semigroup, typename Base>
template<typename Node>
typename Semigroup::Type WithAncAggr<Semigroup, Base>::AggrAnc(Node* node) {
  typedef WithAncAggr Self;
  typename Semigroup::Type aggr = Semigroup::empty_aggr();
  for (; node; node = node->parent()) {
    aggr = Semigroup::CombineAggrs(aggr, node->Self::singleton_aggr());
  }
  return aggr;
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
inline Node* WithAncAggr<Semigroup, Base>::FindDirmostAnc(
    int dir,
    Node* node,
    const Predicate& predicate) {
  return (dir == kLeaf ?
          FindLeafmostAnc(node, predicate) :
          FindRootmostAnc(node, predicate));
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
Node* WithAncAggr<Semigroup, Base>::FindLeafmostAnc(
    Node* node,
    const Predicate& predicate) {
  typedef WithAncAggr Self;
  typename Semigroup::Type cum_aggr = Semigroup::empty_aggr();
  for (; node; node = node->parent()) {
    cum_aggr = Semigroup::CombineAggrs(cum_aggr, node->Self::singleton_aggr());
    if (predicate(cum_aggr)) return node;
  }
  return NULL;
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
Node* WithAncAggr<Semigroup, Base>::FindRootmostAnc(
    Node* node,
    const Predicate& predicate) {
  typedef WithAncAggr Self;
  if (!node) return NULL;
  Node* c = NULL;
  do {
    Node* p = node->parent();
    node->set_parent(c);
    c = node;
    node = p;
  } while (node);
  node = c;
  c = NULL;
  typename Semigroup::Type cum_aggr = Semigroup::empty_aggr();
  while (true) {
    cum_aggr = Semigroup::CombineAggrs(cum_aggr, node->Self::singleton_aggr());
    if (predicate(cum_aggr)) break;
    Node* p = node->parent();
    node->set_parent(c);
    c = node;
    node = p;
    if (!node) return NULL;
  }
  Node* result = node;
  do {
    Node* p = node->parent();
    node->set_parent(c);
    c = node;
    node = p;
  } while (node);
  return result;
}

template<typename Semigroup, typename Base>
template<typename Node>
inline typename Semigroup::Type WithAncAggr<Semigroup, Base>::AggrProperAnc(
    Node* node) {
  return AggrAnc(Parent(node));
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
inline Node* WithAncAggr<Semigroup, Base>::FindDirmostProperAnc(
    int dir,
    Node* node,
    const Predicate& predicate) {
  return FindDirmostAnc(dir, Parent(node), predicate);
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
inline Node* WithAncAggr<Semigroup, Base>::FindLeafmostProperAnc(
    Node* node,
    const Predicate& predicate) {
  return FindDirmostProperAnc(kLeaf, node, predicate);
}

template<typename Semigroup, typename Base>
template<typename Node, typename Predicate>
inline Node* WithAncAggr<Semigroup, Base>::FindRootmostProperAnc(
    Node* node,
    const Predicate& predicate) {
  return FindDirmostProperAnc(kRoot, node, predicate);
}

template<typename Base>
template<typename Node>
void WithEvertBy<Base>::Evert(Node* node) {
  typedef WithEvertBy Self;
  Node* c = NULL;
  while (node) {
    node->Self::add_to_value(Group::flip_delta());
    Node* p = node->parent();
    node->set_parent(c);
    c = node;
    node = p;
  }
}

template<typename Base>
template<typename Node>
void WithEvert<Base>::Evert(Node* node) {
  Node* c = NULL;
  while (node) {
    Node* p = node->parent();
    node->set_parent(c);
    c = node;
    node = p;
  }
}
}  // namespace naive
#endif  // DTREE_NAIVE_TREE_INL_H_
