// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_NAIVE_TREE_EXTRA_H_
#define DTREE_NAIVE_TREE_EXTRA_H_

namespace naive {

template<typename Node, typename Visitor>
void EvertByTraversing(Node* node, Visitor* visitor) {
  Node* c = NULL;
  while (node) {
    Node* p = node->parent();
    (*visitor)(node);
    node->set_parent(c);
    c = node;
    node = p;
  }
}

template<typename IteratorLike>
inline void Assemble(IteratorLike /*first*/, IteratorLike /*last*/) {
}

template<typename Node>
inline void Assemble(Node /*node*/[], size_t /*num_nodes*/) {
}

template<typename Node>
inline void CutOneOfMany(Node* node) {
  node->set_parent(NULL);
}
}  // namespace naive
#endif  // DTREE_NAIVE_TREE_EXTRA_H_
