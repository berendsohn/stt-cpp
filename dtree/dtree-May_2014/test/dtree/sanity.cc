// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

#include "dtree/seq.h"
#include "dtree/seq-inl.h"
#include "dtree/tree.h"
#include "dtree/tree-inl.h"
#include "dtree/tree-extra.h"

namespace {

const char* g_argv0;

#define CHECK(var, expr, bad)                                           \
  do {                                                                  \
    (var) = (expr);                                                     \
    if ((var) == (bad)) {                                               \
      fprintf(stderr, "%s: %s: %s\n", g_argv0, #expr, strerror(errno)); \
      exit(EXIT_FAILURE);                                               \
    }                                                                   \
  } while (false)

void AssertDead(void (*Body)()) {
  pid_t pid;
  CHECK(pid, fork(), -1);
  if (pid) {
    int status;
    int ret;
    CHECK(ret, wait(&status), -1);
    assert(!WIFEXITED(status));
  } else {
    FILE* stream;
    CHECK(stream, freopen("/dev/null", "r", stdout), NULL);
    CHECK(stream, freopen("/dev/null", "r", stderr), NULL);
    Body();
    exit(EXIT_SUCCESS);
  }
}

void Die() {
  assert(false);
}

template<typename Node>
void ValueNull() {
  Node* null = NULL;
  Node::Value(null);
}

template<typename Node>
void SetValueNull() {
  Node* null = NULL;
  Node::SetValue(null, -2);
}

void TestGroup() {
  typedef dtree::Add<int> A;
  assert(A::Plus(2, 3) == 5);
  assert(A::Plus(3, 2) == 5);
  assert(A::Minus(2, 3) == -1);
  assert(A::Minus(3, 2) == 1);
  typedef dtree::Nop<std::string> N;
  assert(N::Plus("hello", "world") == std::string("hello"));
  assert(N::Plus("world", "hello") == std::string("world"));
  assert(N::Minus("hello", "world") == std::string("hello"));
  assert(N::Minus("world", "hello") == std::string("world"));
  typedef dtree::Xor<int> X;
  assert(X::Plus(2, 3) == 1);
  assert(X::Plus(3, 2) == 1);
  assert(X::Minus(2, 3) == 1);
  assert(X::Minus(3, 2) == 1);
  assert(X::FlippedFromValue(0) == 0);
  assert(X::FlippedFromValue(1) == 1);
  assert(X::flip_delta() == 1);
}

void TestSemigroup() {
  typedef dtree::Count<int> C;
  assert(C::AggrFromValue(dtree::NoValue()) == 1);
  assert(C::CombineAggrs(2, 3) == 5);
  assert(C::empty_aggr() == 0);
  typedef dtree::CountAndSum<unsigned int, double> Cas;
  typedef Cas::Type Ca;
  assert(Cas::AggrFromValue(42).count == 1);
  assert(Cas::AggrFromValue(42).sum == 42);
  assert(Cas::CombineAggrs(Ca(2, 3), Ca(4, 5)).count == 6);
  assert(Cas::CombineAggrs(Ca(2, 3), Ca(4, 5)).sum == 8);
  assert(Cas::empty_aggr().count == 0);
  assert(Cas::empty_aggr().sum == 0);
  typedef dtree::Max<unsigned int> Mx;
  assert(Mx::AggrFromValue(42) == 42);
  assert(Mx::CombineAggrs(2, 3) == 3);
  assert(Mx::CombineAggrs(3, 2) == 3);
  assert(Mx::empty_aggr() == 0);
  typedef dtree::Min<unsigned int> Mn;
  assert(Mn::AggrFromValue(42) == 42);
  assert(Mn::CombineAggrs(2, 3) == 2);
  assert(Mn::CombineAggrs(3, 2) == 2);
  assert(Mn::empty_aggr() + 1 == 0);
  typedef dtree::Or<unsigned int> O;
  assert(O::AggrFromValue(42) == 42);
  assert(O::CombineAggrs(3, 5) == 7);
  assert(O::CombineAggrs(5, 3) == 7);
  assert(O::empty_aggr() == 0);
  typedef dtree::Sum<int> S;
  assert(S::AggrFromValue(42) == 42);
  assert(S::CombineAggrs(2, 3) == 5);
  assert(S::CombineAggrs(3, 2) == 5);
  assert(S::empty_aggr() == 0);
}

void TestPredicate() {
  assert(dtree::Greater(2)(3));
  assert(!dtree::Greater(3)(2));
  assert(!dtree::Greater(2)(2));
  assert(!dtree::Greater(3)(3));
  assert(dtree::GreaterEqual(2)(3));
  assert(!dtree::GreaterEqual(3)(2));
  assert(dtree::GreaterEqual(2)(2));
  assert(dtree::GreaterEqual(3)(3));
  assert(!dtree::Less(2)(3));
  assert(dtree::Less(3)(2));
  assert(!dtree::Less(2)(2));
  assert(!dtree::Less(3)(3));
  assert(!dtree::LessEqual(2)(3));
  assert(dtree::LessEqual(3)(2));
  assert(dtree::LessEqual(2)(2));
  assert(dtree::LessEqual(3)(3));
  assert(dtree::Nonzero()(42));
  assert(!dtree::Nonzero()(0));
  assert(dtree::NonzeroAnd(2)(3));
  assert(!dtree::NonzeroAnd(2)(5));
}

template<typename Node>
void TestTree() {
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  assert(!SameTree(u, v));
  Link(u, null);
  Link(v, null);
  Link(null, u);
  Link(null, v);
  Link(null, null);
  assert(!SameTree(u, v));
  Link(u, v);
  assert(SameTree(u, v));
  assert(Parent(u) == v);
  Cut(null);
  Cut(u);
  assert(Parent(null) == null);
  assert(Parent(u) == null);
  Link(w, v);
  Link(v, u);
  assert(Root(null) == null);
  assert(Root(w) == u);
  Cut(w);
  assert(Root(w) == w);
  Cut(v);
  Link(u, v);
  Link(w, v);
  assert(LeafmostCommonAnc(u, w) == v);
  assert(LeafmostCommonAnc(u, v) == v);
  Cut(u);
  assert(LeafmostCommonAnc(u, v) == null);
  assert(LeafmostCommonAnc(u, null) == null);
  assert(LeafmostCommonAnc(u, u) == u);
  assert(LeafmostCommonAnc(null, u) == null);
  assert(LeafmostCommonAnc(null, null) == null);
}

void TestTreeWithDesc() {
  typedef dtree::EndTreeWithDesc<dtree::Begin<> > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  assert(Leaf(w) == u);
  assert(Leaf(v) == u);
  assert(Child(w) == v);
  assert(Child(v) == u);
  ContractDirward(dtree::kRoot, u);
  assert(Parent(u) == null);
  assert(Parent(v) == w);
  Link(u, v);
  ContractDirward(dtree::kLeaf, u);
  assert(Parent(u) == w);
  assert(Parent(v) == null);
  Cut(u);
  Link(u, v);
  Link(v, w);
  ContractRootward(u);
  assert(Parent(u) == null);
  assert(Parent(v) == w);
  Link(u, v);
  ContractLeafward(u);
  assert(Parent(u) == w);
  assert(Parent(v) == null);
  ContractDirward(dtree::kLeaf, null);
  ContractDirward(dtree::kRoot, null);
  ContractLeafward(null);
  ContractRootward(null);
}

template<typename Node>
void TestEvert() {
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  assert(Root(u) == w);
  Node::Evert(u);
  assert(Root(u) == u);
  Node::Evert(null);
  assert(Root(u) == u);
  Node::Evert(v);
  assert(Root(u) == v);
}

void TestAncValue() {
  typedef dtree::EndTree<
      dtree::WithAncValue<dtree::Add<int>, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  Node::AddToAnc(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  Node::AddToProperAnc(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 2);
  Node::AddToAnc(null, 42);
  Node::AddToProperAnc(null, 42);
  Node::SubtractFromProperAnc(null, 24);
  Node::SubtractFromAnc(null, 24);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 2);
  Node::SubtractFromProperAnc(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  Node::SubtractFromAnc(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  Node::SetValue(v, 42);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 42);
  assert(Node::Value(w) == 0);
}

void TestDescValue() {
  typedef dtree::EndTree<
      dtree::WithDescValue<dtree::Add<int>, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node x_;
  Node* x = &x_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  Link(x, w);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  assert(Node::Value(x) == 0);
  Node::AddToTree(v, 1);
  assert(Node::Value(u) == 1);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::AddToDesc(v, 1);
  assert(Node::Value(u) == 2);
  assert(Node::Value(v) == 2);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::AddToTree(null, 42);
  Node::AddToDesc(null, 42);
  Node::SubtractFromDesc(null, 24);
  Node::SubtractFromTree(null, 24);
  assert(Node::Value(u) == 2);
  assert(Node::Value(v) == 2);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::SubtractFromDesc(v, 1);
  assert(Node::Value(u) == 1);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::SubtractFromTree(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  assert(Node::Value(x) == 0);
}

void TestDescValueWithDesc() {
  typedef dtree::EndTreeWithDesc<
      dtree::WithDescValue<dtree::Add<int>, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node x_;
  Node* x = &x_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  Link(x, w);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  assert(Node::Value(x) == 0);
  Node::AddToTree(v, 1);
  assert(Node::Value(u) == 1);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::AddToDesc(v, 1);
  assert(Node::Value(u) == 2);
  assert(Node::Value(v) == 2);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::AddToProperDesc(v, 1);
  assert(Node::Value(u) == 3);
  assert(Node::Value(v) == 2);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::AddToTree(null, 42);
  Node::AddToDesc(null, 42);
  Node::AddToProperDesc(null, 42);
  Node::SubtractFromProperDesc(null, 24);
  Node::SubtractFromDesc(null, 24);
  Node::SubtractFromTree(null, 24);
  assert(Node::Value(u) == 3);
  assert(Node::Value(v) == 2);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::SubtractFromProperDesc(v, 1);
  assert(Node::Value(u) == 2);
  assert(Node::Value(v) == 2);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::SubtractFromDesc(v, 1);
  assert(Node::Value(u) == 1);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  assert(Node::Value(x) == 1);
  Node::SubtractFromTree(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  assert(Node::Value(x) == 0);
  Node::SetValue(v, 42);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 42);
  assert(Node::Value(w) == 0);
  assert(Node::Value(x) == 0);
}

void TestAncAggr() {
  typedef dtree::EndTree<
      dtree::WithAncAggr<dtree::Or<unsigned int>,
      dtree::WithAncValue<dtree::Nop<unsigned int>,
      dtree::Begin<> > > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node x_;
  Node* x = &x_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  Link(x, w);
  Node::SetValue(u, 1);
  Node::SetValue(v, 2);
  Node::SetValue(w, 4);
  Node::SetValue(x, 8);
  assert(Node::AggrAnc(v) == 6);
  assert(Node::AggrAnc(null) == 0);
  assert(Node::AggrProperAnc(v) == 4);
  assert(Node::AggrProperAnc(null) == 0);
  assert(Node::FindDirmostAnc(dtree::kLeaf, v, dtree::Nonzero()) == v);
  assert(Node::FindDirmostAnc(dtree::kLeaf, v, dtree::NonzeroAnd(16u)) ==
         null);
  assert(Node::FindDirmostAnc(dtree::kLeaf, null, dtree::Nonzero()) == null);
  assert(Node::FindLeafmostAnc(v, dtree::Nonzero()) == v);
  assert(Node::FindLeafmostAnc(v, dtree::NonzeroAnd(16u)) == null);
  assert(Node::FindLeafmostAnc(null, dtree::Nonzero()) == null);
  assert(Node::FindDirmostAnc(dtree::kRoot, v, dtree::Nonzero()) == w);
  assert(Node::FindDirmostAnc(dtree::kRoot, v, dtree::NonzeroAnd(16u)) ==
         null);
  assert(Node::FindDirmostAnc(dtree::kRoot, null, dtree::Nonzero()) == null);
  assert(Node::FindRootmostAnc(v, dtree::Nonzero()) == w);
  assert(Node::FindRootmostAnc(v, dtree::NonzeroAnd(16u)) == null);
  assert(Node::FindRootmostAnc(null, dtree::Nonzero()) == null);
  assert(Node::FindDirmostProperAnc(dtree::kLeaf, u, dtree::Nonzero()) == v);
  assert(Node::FindDirmostProperAnc(dtree::kLeaf, u, dtree::NonzeroAnd(16u)) ==
         null);
  assert(Node::FindDirmostProperAnc(dtree::kLeaf, null, dtree::Nonzero()) ==
         null);
  assert(Node::FindLeafmostProperAnc(u, dtree::Nonzero()) == v);
  assert(Node::FindLeafmostProperAnc(u, dtree::NonzeroAnd(16u)) == null);
  assert(Node::FindLeafmostProperAnc(null, dtree::Nonzero()) == null);
  assert(Node::FindDirmostProperAnc(dtree::kRoot, u, dtree::Nonzero()) == w);
  assert(Node::FindDirmostProperAnc(dtree::kRoot, u, dtree::NonzeroAnd(16u)) ==
         null);
  assert(Node::FindDirmostProperAnc(dtree::kRoot, null, dtree::Nonzero()) ==
         null);
  assert(Node::FindRootmostProperAnc(u, dtree::Nonzero()) == w);
  assert(Node::FindRootmostProperAnc(u, dtree::NonzeroAnd(16u)) == null);
  assert(Node::FindRootmostProperAnc(null, dtree::Nonzero()) == null);
}

template<typename Node>
void TestDescAggr() {
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node x_;
  Node* x = &x_;
  Node* null = NULL;
  Link(u, v);
  Link(v, w);
  Link(x, w);
  Node::SetValue(u, 1);
  Node::SetValue(v, 2);
  Node::SetValue(w, 4);
  Node::SetValue(x, 8);
  assert(Node::AggrTree(v) == 15);
  assert(Node::AggrDesc(v) == 3);
  assert(Node::AggrProperDesc(v) == 1);
  assert(Node::FindDirmostTree(dtree::kLeaf, v, dtree::NonzeroAnd(7u)) == u);
  assert(Node::FindDirmostTree(dtree::kLeaf, v, dtree::NonzeroAnd(8u)) == x);
  assert(Node::FindDirmostTree(dtree::kRoot, v, dtree::Nonzero()) == w);
  assert(Node::FindDirmostTree(dtree::kLeaf, null, dtree::Nonzero()) == null);
  assert(Node::FindDirmostTree(dtree::kRoot, null, dtree::Nonzero()) == null);
  assert(Node::FindLeafmostTree(v, dtree::NonzeroAnd(7u)) == u);
  assert(Node::FindLeafmostTree(v, dtree::NonzeroAnd(8u)) == x);
  assert(Node::FindRootmostTree(v, dtree::Nonzero()) == w);
  assert(Node::FindDirmostDesc(dtree::kLeaf, v, dtree::Nonzero()) == u);
  assert(Node::FindDirmostDesc(dtree::kRoot, v, dtree::Nonzero()) == v);
  assert(Node::FindDirmostDesc(dtree::kLeaf, null, dtree::Nonzero()) == null);
  assert(Node::FindDirmostDesc(dtree::kRoot, null, dtree::Nonzero()) == null);
  assert(Node::FindLeafmostDesc(v, dtree::Nonzero()) == u);
  assert(Node::FindRootmostDesc(v, dtree::Nonzero()) == v);
  assert(Node::FindDirmostProperDesc(dtree::kLeaf, v, dtree::Nonzero()) == u);
  assert(Node::FindDirmostProperDesc(dtree::kRoot, v, dtree::Nonzero()) == u);
  assert(Node::FindDirmostProperDesc(dtree::kLeaf, null, dtree::Nonzero()) ==
         null);
  assert(Node::FindDirmostProperDesc(dtree::kRoot, null, dtree::Nonzero()) ==
         null);
  assert(Node::FindLeafmostProperDesc(v, dtree::Nonzero()) == u);
  assert(Node::FindRootmostProperDesc(v, dtree::Nonzero()) == u);
  AssertDead(ValueNull<Node>);
  AssertDead(SetValueNull<Node>);
}

template<typename Node>
class Forwarder {
 public:
  Node* operator()(Node* lhs, const Node* rhs, Node* ptr) {
    return lhs + (ptr - rhs);
  }
};

enum Scope {
  kTree,
  kDesc,
  kProperDesc
};

template<typename Node, typename Predicate>
void TestFindTreeByIndex1(int dir, Scope scope) {
  static const int kBase = 8;
  static const int kNumNodes = ((kBase + 1) * kBase + 1) * kBase + 1;
  int order[kNumNodes];
  Node node[kNumNodes];
  Node copy[kNumNodes];
  std::fill(order, order + kNumNodes, -1);
  for (int i = 1; i < kNumNodes; ++i) Link(&node[i], &node[(i - 1) / kBase]);
  int upper;
  switch (scope) {
    case kTree:
      upper = kNumNodes;
      break;
    case kDesc:
      upper = (kNumNodes - 1) / kBase;
      break;
    case kProperDesc:
      upper = (kNumNodes - 1) / kBase - 1;
      break;
    default:
      assert(false);
  }
  for (int i = 0; i < 2 * upper; ++i) {
    for (int j = 0; j < kNumNodes; ++j) {
      copy[j].CopyFrom(node[j], Forwarder<Node>());
    }
    Node* u;
    switch (scope) {
      case kTree:
        u = Node::FindDirmostTree(dir, &copy[1], Predicate(i));
        break;
      case kDesc:
        u = Node::FindDirmostDesc(dir, &copy[1], Predicate(i));
        break;
      case kProperDesc:
        u = Node::FindDirmostProperDesc(dir, &copy[1], Predicate(i));
        break;
      default:
        assert(false);
    }
    if (u) {
      assert(i < upper);
      int k = u - copy;
      assert(order[k] == -1);
      order[k] = i;
    } else {
      assert(i >= upper);
    }
  }
  for (int i = 0; i < kNumNodes; ++i) {
    for (int j = 0; j < kNumNodes; ++j) {
      if (LeafmostCommonAnc(&node[i], &node[j]) == &node[j]) {
        if (dir == dtree::kLeaf) {
          assert(order[i] <= order[j] || order[j] == -1);
        } else if (dir == dtree::kRoot) {
          assert(order[i] >= order[j]);
        } else {
          assert(false);
        }
      }
    }
  }
}

template<typename Node, typename Predicate>
void TestFindTreeByIndex() {
  TestFindTreeByIndex1<Node, Predicate>(dtree::kLeaf, kTree);
  TestFindTreeByIndex1<Node, Predicate>(dtree::kLeaf, kDesc);
  TestFindTreeByIndex1<Node, Predicate>(dtree::kLeaf, kProperDesc);
  TestFindTreeByIndex1<Node, Predicate>(dtree::kRoot, kTree);
  TestFindTreeByIndex1<Node, Predicate>(dtree::kRoot, kDesc);
  TestFindTreeByIndex1<Node, Predicate>(dtree::kRoot, kProperDesc);
}

template<typename Node>
void LinkConnected() {
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Link(u, v);
  Link(v, u);
}

void TestSeq() {
  typedef dtree::EndSeq<dtree::Begin<> > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  LinkDirOf(dtree::kLeft, u, v);
  assert(Leftward(null) == null);
  assert(Rightward(null) == null);
  assert(Leftward(u) == null);
  assert(Rightward(u) == v);
  assert(Leftward(v) == u);
  assert(Rightward(v) == null);
  assert(Leftward(w) == null);
  assert(Rightward(w) == null);
  assert(SameSeq(null, null));
  assert(!SameSeq(null, u));
  assert(!SameSeq(null, v));
  assert(!SameSeq(null, w));
  assert(!SameSeq(u, null));
  assert(SameSeq(u, u));
  assert(SameSeq(u, v));
  assert(!SameSeq(u, w));
  assert(!SameSeq(v, null));
  assert(SameSeq(v, u));
  assert(SameSeq(v, v));
  assert(!SameSeq(v, w));
  assert(!SameSeq(w, null));
  assert(!SameSeq(w, u));
  assert(!SameSeq(w, v));
  assert(SameSeq(w, w));
  CutDirOf(dtree::kRight, u);
  LinkDirOf(dtree::kRight, u, v);
  assert(Dirward(dtree::kLeft, null) == null);
  assert(Dirward(dtree::kRight, null) == null);
  assert(Dirward(dtree::kLeft, u) == v);
  assert(Dirward(dtree::kRight, u) == null);
  assert(Dirward(dtree::kLeft, v) == null);
  assert(Dirward(dtree::kRight, v) == u);
  CutRightOf(v);
  LinkLeftOf(u, v);
  assert(Dirward(dtree::kLeft, null) == null);
  assert(Dirward(dtree::kRight, null) == null);
  assert(Dirward(dtree::kLeft, u) == null);
  assert(Dirward(dtree::kRight, u) == v);
  assert(Dirward(dtree::kLeft, v) == u);
  assert(Dirward(dtree::kRight, v) == null);
  CutLeftOf(v);
  CutDirOf(dtree::kLeft, u);
  LinkLeftOf(u, v);
  LinkRightOf(w, v);
  LinkDirOf(dtree::kLeft, null, null);
  LinkDirOf(dtree::kLeft, u, null);
  LinkDirOf(dtree::kLeft, null, u);
  LinkDirOf(dtree::kRight, null, null);
  LinkDirOf(dtree::kRight, u, null);
  LinkDirOf(dtree::kRight, null, u);
  assert(Left(w) == u);
  assert(Right(u) == w);
  assert(Dir(dtree::kLeft, null) == null);
  assert(Dir(dtree::kRight, null) == null);
  assert(Dir(dtree::kLeft, u) == u);
  assert(Dir(dtree::kRight, u) == w);
  assert(Dir(dtree::kLeft, w) == u);
  assert(Dir(dtree::kRight, w) == w);
  {
    dtree::ScopedCutDirOf<Node> cut(dtree::kLeft, v);
    assert(cut.dir() == dtree::kLeft);
    assert(cut.excl_part() == u);
    assert(cut.incl_part() == v);
    assert(Left(w) == v);
  }
  assert(Left(w) == u);
  {
    dtree::ScopedCutLeftOf<Node> cut(w);
    assert(Right(u) == v);
  }
  assert(Right(u) == w);
  {
    dtree::ScopedCutDirOf<Node> cut(dtree::kRight, v);
    assert(cut.dir() == dtree::kRight);
    assert(cut.excl_part() == w);
    assert(cut.incl_part() == v);
    assert(Right(u) == v);
  }
  assert(Right(u) == w);
  {
    dtree::ScopedCutRightOf<Node> cut(u);
    assert(Left(w) == v);
  }
  assert(Left(w) == u);
}

void TestReverse() {
  typedef dtree::EndSeq<dtree::WithReverse<dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node* null = NULL;
  LinkLeftOf(u, v);
  assert(Leftward(u) == null);
  assert(Rightward(u) == v);
  assert(Leftward(v) == u);
  assert(Rightward(v) == null);
  Node::ReverseSeq(u);
  assert(Leftward(u) == v);
  assert(Rightward(u) == null);
  assert(Leftward(v) == null);
  assert(Rightward(v) == u);
  Node::ReverseSeq(null);
  assert(Leftward(u) == v);
  assert(Rightward(u) == null);
  assert(Leftward(v) == null);
  assert(Rightward(v) == u);
  Node::ReverseSeq(v);
  assert(Leftward(u) == null);
  assert(Rightward(u) == v);
  assert(Leftward(v) == u);
  assert(Rightward(v) == null);
}

void TestValue() {
  typedef dtree::EndSeq<
      dtree::WithValue<dtree::Add<int>, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  LinkLeftOf(u, v);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  Node::AddToSeq(null, 42);
  Node::AddToSeq(u, 1);
  assert(Node::Value(u) == 1);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 0);
  Node::AddToSeq(w, 1);
  assert(Node::Value(u) == 1);
  assert(Node::Value(v) == 1);
  assert(Node::Value(w) == 1);
  LinkLeftOf(v, w);
  Node::SubtractFromSeq(null, 24);
  Node::SubtractFromSeq(v, 1);
  assert(Node::Value(u) == 0);
  assert(Node::Value(v) == 0);
  assert(Node::Value(w) == 0);
  AssertDead(ValueNull<Node>);
  AssertDead(SetValueNull<Node>);
}

void TestAggr() {
  typedef dtree::EndSeq<
      dtree::WithAggr<dtree::Count<int>, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node* null = NULL;
  assert(Node::AggrSeq(null) == 0);
  assert(Node::AggrSeq(u) == 1);
  assert(Node::AggrSeq(v) == 1);
  assert(Node::AggrSeq(w) == 1);
  LinkLeftOf(u, v);
  assert(Node::AggrSeq(null) == 0);
  assert(Node::AggrSeq(u) == 2);
  assert(Node::AggrSeq(v) == 2);
  assert(Node::AggrSeq(w) == 1);
  LinkLeftOf(v, w);
  assert(Node::AggrSeq(null) == 0);
  assert(Node::AggrSeq(u) == 3);
  assert(Node::AggrSeq(v) == 3);
  assert(Node::AggrSeq(w) == 3);
  assert(Node::FindDirmostSeq(dtree::kLeft, u, dtree::Index(0)) == u);
  assert(Node::FindLeftmostSeq(u, dtree::Index(1)) == v);
  assert(Node::FindLeftmostSeq(u, dtree::Index(2)) == w);
  assert(Node::FindLeftmostSeq(u, dtree::Index(3)) == null);
  assert(Node::FindDirmostSeq(dtree::kRight, u, dtree::Index(0)) == w);
  assert(Node::FindRightmostSeq(u, dtree::Index(1)) == v);
  assert(Node::FindRightmostSeq(u, dtree::Index(2)) == u);
  assert(Node::FindRightmostSeq(u, dtree::Index(3)) == null);
}

void TestSum() {
  typedef dtree::EndSeq<
      dtree::WithAggr<dtree::Sum<int>,
      dtree::WithValue<dtree::Nop<int>,
      dtree::Begin<> > > > Node;
  static const int kNumNodes = 100;
  Node node[kNumNodes];
  for (int i = 1; i < kNumNodes; ++i) LinkLeftOf(&node[i - 1], &node[i]);
  for (int i = 0; i < kNumNodes; ++i) Node::SetValue(&node[i], 2 * i + 1);
  for (int j = 0; j < 2 * kNumNodes * kNumNodes; ++j) {
    Node* u = Node::FindLeftmostSeq(&node[0], dtree::Index(j));
    if (u) {
      int i = u - node;
      assert(i * i <= j && j < (i + 1) * (i + 1));
    } else {
      assert(j >= kNumNodes * kNumNodes);
    }
  }
}

void TestCountAndSum() {
  typedef dtree::EndSeq<
      dtree::WithAggr<dtree::CountAndSum<int, int>,
      dtree::WithValue<dtree::Add<int>,
      dtree::Begin<> > > > Node;
  static const int kNumNodes = 100;
  Node node[kNumNodes];
  for (int i = 1; i < kNumNodes; ++i) LinkLeftOf(&node[i - 1], &node[i]);
  Node::AddToSeq(&node[0], 1);
  for (int i = 1; i < kNumNodes; ++i) {
    dtree::ScopedCutLeftOf<Node> cut(&node[i]);
    Node::AddToSeq(&node[i], 2);
  }
  for (int j = 0; j < 2 * kNumNodes * kNumNodes; ++j) {
    Node* u = Node::FindLeftmostSeq(&node[0], dtree::IndexBySum(j));
    if (u) {
      int i = u - node;
      assert(i * i <= j && j < (i + 1) * (i + 1));
      assert(Node::FindLeftmostSeq(&node[0], dtree::IndexByCount(i)) == u);
    } else {
      assert(j >= kNumNodes * kNumNodes);
    }
  }
}

void LinkDirOfConnected() {
  typedef dtree::EndSeq<dtree::Begin<> > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  LinkLeftOf(u, v);
  LinkLeftOf(v, w);
  LinkLeftOf(w, u);
}

template<typename Predicate>
void NegativeIndex() {
  Predicate(-1);
}

void TestDirectedPair() {
  {
    typedef dtree::EndTree<
        dtree::WithAncAggr<dtree::DpMax<int>,
        dtree::WithEvertBy<
        dtree::WithAncValue<
        dtree::DpAdd<int>,
        dtree::Begin<> > > > > Node;
    Node u_;
    Node* u = &u_;
    Node v_;
    Node* v = &v_;
    Node w_;
    Node* w = &w_;
    Node* null = NULL;
    Link(u, v);
    Link(v, w);
    dtree::DpValue<int> x;
    x.set_lw(42);
    x.set_rw(24);
    Node::AddToAnc(u, x);
    assert(Node::Value(u).lw() == 42);
    assert(Node::Value(u).rw() == 24);
    assert(Node::Value(v).lw() == 42);
    assert(Node::Value(v).rw() == 24);
    assert(Node::Value(w).lw() == 42);
    assert(Node::Value(w).rw() == 24);
    assert(Parent(u) == v);
    assert(Parent(v) == w);
    assert(Parent(w) == null);
    Node::Evert(v);
    assert(Node::Value(u).dw(dtree::kLw) == 42);
    assert(Node::Value(u).dw(dtree::kRw) == 24);
    assert(Node::Value(v).dw(dtree::kLw) == 24);
    assert(Node::Value(v).dw(dtree::kRw) == 42);
    assert(Node::Value(w).dw(dtree::kLw) == 24);
    assert(Node::Value(w).dw(dtree::kRw) == 42);
    assert(Parent(u) == v);
    assert(Parent(v) == null);
    assert(Parent(w) == v);
    dtree::DpAggr<int> y = Node::AggrAnc(u);
    assert(y.lw() == 42);
    assert(y.rw() == 42);
    y = Node::AggrAnc(w);
    assert(y.dw(dtree::kLw) == 24);
    assert(y.dw(dtree::kRw) == 42);
  }
  {
    typedef dtree::EndSeq<
        dtree::WithAggr<dtree::DpMin<int>,
        dtree::WithReverseBy<
        dtree::WithValue<dtree::DpAdd<int>,
        dtree::Begin<> > > > > Node;
    Node u_;
    Node* u = &u_;
    Node v_;
    Node* v = &v_;
    Node* null = NULL;
    LinkLeftOf(u, v);
    Node::SetValue(u, dtree::DpValue<int>(2, 3));
    Node::SetValue(v, dtree::DpValue<int>(42, 24));
    assert(Node::Value(u).lw() == 2);
    assert(Node::Value(u).rw() == 3);
    assert(Node::Value(v).lw() == 42);
    assert(Node::Value(v).rw() == 24);
    assert(Node::AggrSeq(u).lw() == 2);
    assert(Node::AggrSeq(u).rw() == 3);
    Node::ReverseSeq(u);
    assert(Node::Value(u).lw() == 3);
    assert(Node::Value(u).rw() == 2);
    assert(Node::Value(v).lw() == 24);
    assert(Node::Value(v).rw() == 42);
    assert(Leftward(u) == v);
    assert(Rightward(u) == null);
    assert(Leftward(v) == null);
    assert(Rightward(v) == u);
    Node::AggrSeq(null);
  }
  {
    dtree::DpAggr<int> a;
    assert(a.lw() == 0);
    assert(a.rw() == 0);
    a.set_lw(42);
    a.set_rw(24);
    assert(a.dw(dtree::kLw) == 42);
    assert(a.dw(dtree::kRw) == 24);
    a.set_dw(dtree::kLw, 24);
    a.set_dw(dtree::kRw, 42);
    assert(a.lw() == 24);
    assert(a.rw() == 42);
    dtree::DpAggr<int> b(2, 3);
    assert(b.lw() == 2);
    assert(b.rw() == 3);
    dtree::DpValue<int> x;
    assert(x.lw() == 0);
    assert(x.rw() == 0);
    x.set_lw(42);
    x.set_rw(24);
    assert(x.dw(dtree::kLw) == 42);
    assert(x.dw(dtree::kRw) == 24);
    x.set_dw(dtree::kLw, 24);
    x.set_dw(dtree::kRw, 42);
    assert(x.lw() == 24);
    assert(x.rw() == 42);
    dtree::DpValue<int> y(4, 5);
    assert(y.lw() == 4);
    assert(y.rw() == 5);
    assert(dtree::DwGreater(dtree::kLw, 1)(b));
    assert(!dtree::DwGreater(dtree::kLw, 2)(b));
    assert(dtree::DwGreater(dtree::kRw, 2)(b));
    assert(dtree::DwGreaterEqual(dtree::kLw, 1)(b));
    assert(dtree::DwGreaterEqual(dtree::kLw, 2)(b));
    assert(!dtree::DwGreaterEqual(dtree::kLw, 3)(b));
    assert(dtree::DwGreaterEqual(dtree::kRw, 3)(b));
    assert(!dtree::DwGreaterEqual(dtree::kRw, 4)(b));
    assert(dtree::DwLess(dtree::kLw, 4)(b));
    assert(dtree::DwLess(dtree::kLw, 3)(b));
    assert(!dtree::DwLess(dtree::kLw, 2)(b));
    assert(dtree::DwLessEqual(dtree::kRw, 4)(b));
    assert(dtree::DwLessEqual(dtree::kRw, 3)(b));
    assert(!dtree::DwLessEqual(dtree::kRw, 2)(b));
    assert(dtree::LwGreater(1)(b));
    assert(!dtree::LwGreater(2)(b));
    assert(!dtree::LwGreater(3)(b));
    assert(dtree::LwGreaterEqual(1)(b));
    assert(dtree::LwGreaterEqual(2)(b));
    assert(!dtree::LwGreaterEqual(3)(b));
    assert(dtree::LwLess(4)(b));
    assert(dtree::LwLess(3)(b));
    assert(!dtree::LwLess(2)(b));
    assert(!dtree::LwLess(1)(b));
    assert(dtree::LwLessEqual(4)(b));
    assert(dtree::LwLessEqual(3)(b));
    assert(dtree::LwLessEqual(2)(b));
    assert(!dtree::LwLessEqual(1)(b));
    assert(!dtree::LwLessEqual(0)(b));
    assert(dtree::RwGreater(1)(b));
    assert(dtree::RwGreater(2)(b));
    assert(!dtree::RwGreater(3)(b));
    assert(!dtree::RwGreater(4)(b));
    assert(dtree::RwGreaterEqual(1)(b));
    assert(dtree::RwGreaterEqual(2)(b));
    assert(dtree::RwGreaterEqual(3)(b));
    assert(!dtree::RwGreaterEqual(4)(b));
    assert(!dtree::RwGreaterEqual(5)(b));
    assert(!dtree::RwLess(2)(b));
    assert(!dtree::RwLess(3)(b));
    assert(dtree::RwLess(4)(b));
    assert(dtree::RwLess(5)(b));
    assert(!dtree::RwLessEqual(2)(b));
    assert(dtree::RwLessEqual(3)(b));
    assert(dtree::RwLessEqual(4)(b));
  }
}
struct IntPair {
  IntPair();
  IntPair(int a_, int b_);
  int a;
  int b;
};

IntPair::IntPair() : a(0), b(0) {
}

IntPair::IntPair(int a_, int b_) : a(a_), b(b_) {
}

inline bool operator==(IntPair x, IntPair y) {
  return x.a == y.a && x.b == y.b;
}

struct IntPairAddNop {
  typedef IntPair Type;
  static IntPair Plus(IntPair x, IntPair y) { return IntPair(x.a + y.a, x.b); }
  static IntPair Minus(IntPair x, IntPair y) {
    return IntPair(x.a - y.a, x.b);
  }
};

void TestSetNonDescValue() {
  typedef dtree::EndTree<
      dtree::WithDescValue<IntPairAddNop, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node x_;
  Node* x = &x_;
  Node y_;
  Node* y = &y_;
  Link(u, x);
  Link(v, x);
  Link(w, x);
  Link(x, y);
  typedef IntPair IP;
  assert(Node::Value(u) == IP(0, 0));
  assert(Node::Value(v) == IP(0, 0));
  assert(Node::Value(w) == IP(0, 0));
  assert(Node::Value(x) == IP(0, 0));
  assert(Node::Value(y) == IP(0, 0));
  Node::AddToDesc(u, IP(256, 512));
  Node::AddToDesc(v, IP(64, 128));
  Node::AddToDesc(w, IP(16, 32));
  Node::AddToDesc(x, IP(4, 8));
  Node::AddToDesc(y, IP(1, 2));
  assert(Node::Value(u) == IP(261, 0));
  assert(Node::Value(v) == IP(69, 0));
  assert(Node::Value(w) == IP(21, 0));
  assert(Node::Value(x) == IP(5, 0));
  assert(Node::Value(y) == IP(1, 0));
  Node::SetNonDescValue(x, IP(1024, 2048));
  assert(Node::Value(u) == IP(261, 0));
  assert(Node::Value(v) == IP(69, 0));
  assert(Node::Value(w) == IP(21, 0));
  assert(Node::Value(x) == IP(5, 2048));
  assert(Node::Value(y) == IP(1, 0));
  Node::SetNonDescValue(x, IP(4096, 8192));
  assert(Node::Value(u) == IP(261, 0));
  assert(Node::Value(v) == IP(69, 0));
  assert(Node::Value(w) == IP(21, 0));
  assert(Node::Value(x) == IP(5, 8192));
  assert(Node::Value(y) == IP(1, 0));
}

struct Monomial {
  Monomial();
  Monomial(int a, int b);
  int a;
  int b;
};

Monomial::Monomial() : a(0), b(1) {
}

Monomial::Monomial(int a_, int b_) : a(a_), b(b_) {
}

inline bool operator==(Monomial x, Monomial y) {
  return x.a == y.a && x.b == y.b;
}

struct MonomialCompose {
  typedef Monomial Type;
  static Monomial Plus(Monomial x, Monomial y) {
    return Monomial(x.a + x.b * y.a, x.b * y.b);
  }
  static Monomial Minus(Monomial x, Monomial y) {
    return Monomial(x.a - x.b * y.a * y.b, x.b * y.b);
  }
  static Monomial PlusFilter(Monomial x, Monomial y) {
    return Monomial(x.a, x.b * y.b);
  }
  static Monomial MinusFilter(Monomial x, Monomial y) {
    return Monomial(x.a, x.b * y.b);
  }
};

void TestAncDescValue() {
  typedef dtree::EndTreeWithDesc<
      dtree::WithAncDescValue<MonomialCompose, dtree::Begin<> > > Node;
  Node u_;
  Node* u = &u_;
  Node v_;
  Node* v = &v_;
  Node w_;
  Node* w = &w_;
  Node x_;
  Node* x = &x_;
  Node y_;
  Node* y = &y_;
  Link(u, x);
  Link(v, x);
  Link(w, x);
  Link(x, y);
  typedef Monomial M;
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToAnc(x, M(2, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(2, 1));
  assert(Node::Value(y) == M(2, 1));
  Node::AddToAnc(x, M(3, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(5, 1));
  assert(Node::Value(y) == M(5, 1));
  Node::SubtractFromAnc(x, M(2, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(3, 1));
  assert(Node::Value(y) == M(3, 1));
  Node::SubtractFromAnc(x, M(3, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToProperAnc(x, M(4, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(4, 1));
  Node::AddToProperAnc(x, M(5, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(9, 1));
  Node::SubtractFromProperAnc(x, M(4, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(5, 1));
  Node::SubtractFromProperAnc(x, M(5, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToTree(x, M(6, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToTree(x, M(7, -1));
  assert(Node::Value(u) == M(0, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(0, -1));
  assert(Node::Value(x) == M(0, -1));
  assert(Node::Value(y) == M(0, -1));
  Node::SubtractFromTree(x, M(6, 1));
  assert(Node::Value(u) == M(0, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(0, -1));
  assert(Node::Value(x) == M(0, -1));
  assert(Node::Value(y) == M(0, -1));
  Node::SubtractFromTree(x, M(7, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToDesc(x, M(6, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToDesc(x, M(7, -1));
  assert(Node::Value(u) == M(0, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(0, -1));
  assert(Node::Value(x) == M(0, -1));
  assert(Node::Value(y) == M(0, 1));
  Node::SubtractFromDesc(x, M(6, 1));
  assert(Node::Value(u) == M(0, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(0, -1));
  assert(Node::Value(x) == M(0, -1));
  assert(Node::Value(y) == M(0, 1));
  Node::SubtractFromDesc(x, M(7, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToProperDesc(x, M(6, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToProperDesc(x, M(7, -1));
  assert(Node::Value(u) == M(0, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(0, -1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::SubtractFromProperDesc(x, M(6, 1));
  assert(Node::Value(u) == M(0, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(0, -1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::SubtractFromProperDesc(x, M(7, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::SetNonDescValue(x, M(8, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(8, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::SetNonDescValue(x, M(9, -1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(9, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::SetNonDescValue(x, M(0, 1));
  assert(Node::Value(u) == M(0, 1));
  assert(Node::Value(v) == M(0, 1));
  assert(Node::Value(w) == M(0, 1));
  assert(Node::Value(x) == M(0, 1));
  assert(Node::Value(y) == M(0, 1));
  Node::AddToAnc(u, M(2, 1));
  Node::AddToTree(v, M(0, -1));
  Node::AddToAnc(w, M(3, 1));
  assert(Node::Value(u) == M(2, -1));
  assert(Node::Value(v) == M(0, -1));
  assert(Node::Value(w) == M(-3, -1));
  assert(Node::Value(x) == M(-1, -1));
  assert(Node::Value(y) == M(-1, -1));
}
}  // namespace

int main(int /*argc*/, char** argv) {
  g_argv0 = argv[0];
  AssertDead(Die);
  TestGroup();
  TestSemigroup();
  TestPredicate();
  TestTree<dtree::EndTree<dtree::Begin<> > >();
  TestTree<dtree::EndTreeWithDesc<dtree::Begin<> > >();
  TestTreeWithDesc();
  TestEvert<dtree::EndTree<dtree::WithEvert<dtree::Begin<> > > >();
  TestEvert<dtree::EndTreeWithDesc<dtree::WithEvert<dtree::Begin<> > > >();
  TestAncValue();
  TestDescValue();
  TestDescValueWithDesc();
  TestAncAggr();
  {
    typedef dtree::EndTreeWithDesc<
        dtree::WithDescAggr<dtree::Or<unsigned int>,
        dtree::WithAncValue<dtree::Nop<unsigned int>,
        dtree::Begin<> > > > Node1;
    TestDescAggr<Node1>();
    typedef dtree::EndTreeWithDesc<
        dtree::WithDescAggr<dtree::Or<unsigned int>,
        dtree::WithDescValue<dtree::Nop<unsigned int>,
        dtree::Begin<> > > > Node2;
    TestDescAggr<Node2>();
  }
  {
    typedef dtree::EndTreeWithDesc<
        dtree::WithDescAggr<dtree::Count<int>,
        dtree::Begin<> > > Node1;
    TestFindTreeByIndex<Node1, dtree::Index_<int> >();
    typedef dtree::EndTreeWithDesc<
        dtree::WithDescAggr<dtree::CountAndSum<int, int>,
        dtree::WithAncValue<dtree::Add<int>,
        dtree::Begin<> > > > Node2;
    TestFindTreeByIndex<Node2, dtree::IndexByCount_<int> >();
    typedef dtree::EndTreeWithDesc<
        dtree::WithDescAggr<dtree::CountAndSum<int, int>,
        dtree::WithDescValue<dtree::Add<int>,
        dtree::Begin<> > > > Node3;
    TestFindTreeByIndex<Node3, dtree::IndexByCount_<int> >();
  }
  AssertDead(LinkConnected<dtree::EndTree<dtree::Begin<> > >);
  AssertDead(LinkConnected<dtree::EndTreeWithDesc<dtree::Begin<> > >);
  TestSeq();
  AssertDead(LinkDirOfConnected);
  TestReverse();
  TestValue();
  TestAggr();
  TestSum();
  TestCountAndSum();
  AssertDead(NegativeIndex<dtree::Index_<int> >);
  AssertDead(NegativeIndex<dtree::IndexByCount_<int> >);
  AssertDead(NegativeIndex<dtree::IndexBySum_<int> >);
  TestDirectedPair();
  TestSetNonDescValue();
  TestAncDescValue();
  printf("OK %s\n", argv[0]);
}
