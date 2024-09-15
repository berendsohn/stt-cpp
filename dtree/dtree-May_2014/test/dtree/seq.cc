// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include <stdio.h>
#include <vector>

#include "dtree/naive/seq.h"
#include "dtree/naive/seq-inl.h"
#include "dtree/selfadjust/seq.h"
#include "dtree/selfadjust/seq-inl.h"
#include "util/random.h"
#include "util/scoped.h"

#include "test/dtree/common.cc"

namespace {

template<typename Group, typename Base>
class WithValue {
 public:
  typedef naive::WithValue<Group, typename Base::SelfA> SelfA;
  typedef selfadjust::WithValue<Group, typename Base::SelfB> SelfB;
  enum {
    kNumOutcomes = 4,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        assert(!a || SelfA::Value(a) == SelfB::Value(b));
        break;
      case 1:
        if (a) {
          typename Group::Type value =
              state->template NextValue<typename Group::Type>();
          SelfA::SetValue(a, value);
          SelfB::SetValue(b, value);
        }
        break;
      case 2:
        {
          typename Group::Type delta =
              state->template NextDelta<typename Group::Type>();
          SelfA::AddToSeq(a, delta);
          SelfB::AddToSeq(b, delta);
        }
        break;
      case 3:
        {
          typename Group::Type delta =
              state->template NextDelta<typename Group::Type>();
          SelfA::SubtractFromSeq(a, delta);
          SelfB::SubtractFromSeq(b, delta);
        }
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
  template<typename NodeA, typename NodeB>
  static void CompareValues(const State<NodeA, NodeB>& state) {
    Base::CompareValues(state);
    for (int i = 1; i < state.num_nodes(); ++i) {
      const NodeB* b = state.node_b(i);
      const NodeB* p = b->parent();
      typename Group::Type value_b;
      if (p) {
        value_b = Group::Plus(b->SelfB::value(),
                              state.corresponding_node_a(p)->SelfA::value());
      } else {
        value_b = b->SelfB::value();
      }
      assert(state.corresponding_node_a(b)->SelfA::value() == value_b);
    }
  }
  template<typename NodeA, typename NodeB>
  static void ValidateAggrs(const State<NodeA, NodeB>& state,
                            const std::vector<const NodeB*>& top_order) {
    Base::ValidateAggrs(state, top_order);
  }
  static void PrintDecorA(const SelfA* a) {
    Base::PrintDecorA(a);
    fputs(" v", stdout);
    Print(a->value());
  }
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    fputs(" v", stdout);
    Print(b->value());
  }
};

template<typename Semigroup, typename Predicate, typename Base>
class WithAggr {
 public:
  typedef naive::WithAggr<Semigroup, typename Base::SelfA> SelfA;
  typedef selfadjust::WithAggr<Semigroup, typename Base::SelfB> SelfB;
  enum {
    kNumOutcomes = 2,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        assert(SelfA::AggrSeq(a) == SelfB::AggrSeq(b));
        break;
      case 1:
        {
          int dir = state->NextDir();
          const Predicate predicate =
              state->template NextPredicate<Predicate>();
          NodeA* a1 = SelfA::FindDirmostSeq(dir, a, predicate);
          NodeB* b1 = SelfB::FindDirmostSeq(dir, b, predicate);
          assert(state->nodes_correspond(a1, b1));
        }
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
  template<typename NodeA, typename NodeB>
  static void CompareValues(const State<NodeA, NodeB>& state) {
    Base::CompareValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void ValidateAggrs(const State<NodeA, NodeB>& state,
                            const std::vector<const NodeB*>& top_order) {
    Base::ValidateAggrs(state, top_order);
    std::vector<typename Semigroup::Type> aggr(state.num_nodes());
    for (int i = 1; i < state.num_nodes(); ++i) {
      aggr[i] = Semigroup::AggrFromValue(state.node_a(i)->SelfA::value());
    }
    for (typename std::vector<const NodeB*>::const_iterator it =
             top_order.begin();
         it != top_order.end();
         ++it) {
      const NodeB* b = *it;
      const NodeB* p = b->parent();
      if (!p) continue;
      int i = state.index_b(b);
      assert(aggr[i] ==
             SelfB::Group_::Plus(b->SelfB::delta_aggr(),
                                 state.node_a(i)->SelfA::value()));
      int k = state.index_b(p);
      aggr[k] = Semigroup::CombineAggrs(aggr[k], aggr[i]);
    }
  }
  static void PrintDecorA(const SelfA* a) { Base::PrintDecorA(a); }
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    fputs(" a", stdout);
    Print(b->delta_aggr());
  }
};

template<typename Base>
class WithReverseBy {
 public:
  typedef naive::WithReverseBy<typename Base::SelfA> SelfA;
  typedef selfadjust::WithReverseBy<typename Base::SelfB> SelfB;
  enum {
    kNumOutcomes = 1,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        SelfA::ReverseSeq(a);
        SelfB::ReverseSeq(b);
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
  template<typename NodeA, typename NodeB>
  static void CompareValues(const State<NodeA, NodeB>& state) {
    Base::CompareValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void ValidateAggrs(const State<NodeA, NodeB>& state,
                            const std::vector<const NodeB*>& top_order) {
    Base::ValidateAggrs(state, top_order);
  }
  static void PrintDecorA(const SelfA* a) { Base::PrintDecorA(a); }
  static void PrintDecorB(const SelfB* b) { Base::PrintDecorB(b); }
};

template<typename Base>
class WithReverse {
 public:
  typedef naive::WithReverse<typename Base::SelfA> SelfA;
  typedef selfadjust::WithReverse<typename Base::SelfB> SelfB;
  enum {
    kNumOutcomes = 1,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        SelfA::ReverseSeq(a);
        SelfB::ReverseSeq(b);
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
  template<typename NodeA, typename NodeB>
  static void CompareValues(const State<NodeA, NodeB>& state) {
    Base::CompareValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void ValidateAggrs(const State<NodeA, NodeB>& state,
                            const std::vector<const NodeB*>& top_order) {
    Base::ValidateAggrs(state, top_order);
  }
  static void PrintDecorA(const SelfA* a) { Base::PrintDecorA(a); }
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    fputs(" f", stdout);
    Print(b->value());
  }
};

////////////////////////////////////////

template<typename Base>
class EndSeq {
 public:
  typedef naive::EndSeq<typename Base::SelfA> NodeA;
  typedef selfadjust::EndSeq<typename Base::SelfB> NodeB;
  enum {
    kNumOutcomes = 5,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  explicit EndSeq(int num_nodes);
  void Step();
  void Validate() const;
  void PrintA() const;
  void PrintB() const;
  void ReallocateAB() { state_.ReallocateAB(); }

 private:
  void AcyclicA() const;
  void AcyclicB() const;
  void TraverseB(const NodeB* b,
                 const NodeB* p,
                 std::vector<bool>* visited) const;
  const NodeB* CompareTopology(const NodeB* b,
                               int flipped,
                               const NodeB* r) const;
  void PrintConcreteTree(const NodeB* b,
                         int indent,
                         char relationship,
                         const NodeB* p) const;
  State<NodeA, NodeB> state_;
  DISALLOW_COPY_AND_ASSIGN(EndSeq);
};

template<typename Base>
EndSeq<Base>::EndSeq(int num_nodes) : state_(num_nodes) {
}

template<typename Base>
void EndSeq<Base>::Step() {
  int outcome = state_.NextOutcome(kTotalOutcomes);
  NodeA* a;
  NodeB* b;
  state_.NextCorrespondingNodes(&a, &b);
  switch (outcome) {
    case 0:
      {
        int dir = state_.NextDir();
        assert(state_.nodes_correspond(Dir(dir, a), Dir(dir, b)));
      }
      break;
    case 1:
      {
        int dir = state_.NextDir();
        NodeA* a1 = CutDirOf(dir, a);
        NodeB* b1 = CutDirOf(dir, b);
        assert(SameSeq(a1, state_.corresponding_node_a(b1)));
        assert(SameSeq(b1, state_.corresponding_node_b(a1)));
        break;
      }
    case 2:
    case 3:
      {
        int dir = state_.NextDir();
        NodeA* a1;
        NodeB* b1;
        state_.NextCorrespondingNodes(&a1, &b1);
        bool connected_a = SameSeq(a, a1);
        bool connected_b = SameSeq(b, b1);
        assert(connected_a == connected_b);
        assert(connected_a ||
               state_.nodes_correspond(LinkDirOf(dir, a, a1),
                                       LinkDirOf(dir, b, b1)));
      }
      break;
    case 4:
      {
        int dir = state_.NextDir();
        assert(state_.nodes_correspond(Dirward(dir, a), Dirward(dir, b)));
      }
      break;
    default:
      Base::Step(outcome - kNumOutcomes, a, b, &state_);
  }
}

template<typename Base>
void EndSeq<Base>::Validate() const {
  AcyclicA();
  AcyclicB();
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    if (!b->parent()) CompareTopology(b, 0, NULL);
  }
  Base::CompareValues(state_);
  std::vector<const NodeB*> top_order;
  state_.OrderNodesTopologically(&top_order);
  Base::ValidateAggrs(state_, top_order);
}

template<typename Base>
void EndSeq<Base>::AcyclicA() const {
  std::vector<bool> visited(state_.num_nodes(), false);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeA* a = state_.node_a(i);
    if (a->solid(naive::kLeft)) continue;
    while (true) {
      int j = state_.index_a(a);
      assert(!visited[j]);
      visited[j] = true;
      const NodeA* a1 = a->solid(naive::kRight);
      if (!a1) break;
      assert(a1->solid(naive::kLeft) == a);
      a = a1;
    }
  }
  for (int i = 1; i < state_.num_nodes(); ++i) assert(visited[i]);
}

template<typename Base>
void EndSeq<Base>::AcyclicB() const {
  std::vector<bool> visited(state_.num_nodes(), false);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    if (!b->parent()) TraverseB(b, NULL, &visited);
  }
  for (int i = 1; i < state_.num_nodes(); ++i) assert(visited[i]);
}

template<typename Base>
void EndSeq<Base>::TraverseB(const NodeB* b,
                             const NodeB* p,
                             std::vector<bool>* visited) const {
  if (!b) return;
  int j = state_.index_b(b);
  assert(!(*visited)[j]);
  (*visited)[j] = true;
  assert(b->parent() == p);
  TraverseB(b->solid(0), b, visited);
  TraverseB(b->solid(1), b, visited);
}

template<typename Base>
const typename EndSeq<Base>::NodeB* EndSeq<Base>::CompareTopology(
    const NodeB* b,
    int flipped,
    const NodeB* r) const {
  if (!b) return r;
  flipped ^= b->SolidFlipped();
  const NodeB* l =
      CompareTopology(b->solid(selfadjust::kRight ^ flipped), flipped, r);
  assert(state_.nodes_correspond(
      state_.corresponding_node_a(b)->solid(naive::kRight), l));
  return CompareTopology(b->solid(selfadjust::kLeft ^ flipped), flipped, b);
}

template<typename Base>
void EndSeq<Base>::PrintA() const {
  bool soft_space = false;
  fputs("<<<\n", stdout);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeA* a = state_.node_a(i);
    if (a->solid(naive::kRight)) continue;
    if (soft_space) putchar('\n');
    do {
      printf("  %d:", state_.index_a(a));
      Base::PrintDecorA(a);
      putchar('\n');
    } while ((a = a->solid(naive::kLeft)));
    soft_space = true;
  }
  fputs(">>>\n", stdout);
}

template<typename Base>
void EndSeq<Base>::PrintB() const {
  bool soft_space = false;
  fputs("<<<\n", stdout);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    if (b->parent()) continue;
    if (soft_space) putchar('\n');
    PrintConcreteTree(b, 3, '@', NULL);
    soft_space = true;
  }
  fputs(">>>\n", stdout);
}

template<typename Base>
void EndSeq<Base>::PrintConcreteTree(const NodeB* b,
                                     int indent,
                                     char relationship,
                                     const NodeB* p) const {
  if (!b) return;
  if (indent > 40) {
    printf("%*c...\n", indent, relationship);
    return;
  }
  PrintConcreteTree(b->solid(1), indent + 2, '/', b);
  printf("%*c%d%c", indent, relationship, state_.index_b(b),
         b->parent() == p ? ':' : '#');
  Base::PrintDecorB(b);
  putchar('\n');
  PrintConcreteTree(b->solid(0), indent + 2, '\\', b);
}
}  // namespace

int main(int /*argc*/, char** argv) {
  static char buf[BUFSIZ];
  setvbuf(stdout, buf, _IOLBF, sizeof buf);
  DTREE_CONFIG test(16);
  test.Validate();
  for (int percent = 0; percent < 100; ++percent) {
    if (false) printf("%2d%%\n", percent);
    for (int i = 0; i < 1000; ++i) {
      test.Step();
      if (false) {
        test.PrintA();
        test.PrintB();
      }
      test.Validate();
    }
  }
  test.ReallocateAB();
  test.Validate();
  test.PrintA();
  test.PrintB();
  if (false) printf("OK %s\n", argv[0]);
}
