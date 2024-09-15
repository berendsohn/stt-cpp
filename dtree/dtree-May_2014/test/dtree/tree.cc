// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include <stdio.h>
#include <vector>

#include "dtree/naive/tree.h"
#include "dtree/naive/tree-inl.h"
#include "dtree/naive/tree-extra.h"
#include "dtree/selfadjust/tree.h"
#include "dtree/selfadjust/tree-inl.h"
#include "dtree/selfadjust/tree-extra.h"
#include "util/random.h"

#include "test/dtree/common.cc"

namespace {

template<typename Type, typename Base>
class TypeStaticValue {
 public:
  typedef naive::WithStaticValue<Type, typename Base::SelfA> SelfA;
  typedef selfadjust::WithStaticValue<Type, typename Base::SelfB> SelfB;
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
};

template<typename Group, typename Base>
class TypeAncValue {
 public:
  typedef naive::WithAncValue<Group, typename Base::SelfA> SelfA;
  typedef selfadjust::WithAncValue<Group, typename Base::SelfB> SelfB;
  class VisitorA : public Base::VisitorA {
   public:
    VisitorA() : child_value_() {}
    template<typename Node>
    void operator()(Node* node) {
      typename Group::Type node_value = node->SelfA::value();
      node->SelfA::set_value(child_value_);
      child_value_ = node_value;
    }
   private:
    typename Group::Type child_value_;
    DISALLOW_COPY_AND_ASSIGN(VisitorA);
  };
  class VisitorB : public Base::VisitorB {
   public:
    VisitorB() : child_value_() {}
    template<typename Node>
    void operator()(Node* node) {
      typename Group::Type node_value = node->SelfB::value();
      node->SelfB::set_value(child_value_);
      child_value_ = node_value;
    }
   private:
    typename Group::Type child_value_;
    DISALLOW_COPY_AND_ASSIGN(VisitorB);
  };
};

template<typename Group, typename Base>
class TypeDescValue {
 public:
  typedef naive::WithAncValue<Group, typename Base::SelfA> SelfA;
  typedef selfadjust::WithDescValue<Group, typename Base::SelfB> SelfB;
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
};

template<typename Group, typename Base>
class TypeAncDescValue {
 public:
  typedef naive::WithAncValue<Group, typename Base::SelfA> SelfA;
  typedef selfadjust::WithAncDescValue<Group, typename Base::SelfB> SelfB;
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
};

template<typename Typist, typename Base>
class CommonValue : public Base {
 public:
  typedef typename Typist::SelfA SelfA;
  typedef typename Typist::SelfB SelfB;
  enum {
    kNumOutcomes = 1,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void CopyValues(State<NodeA, NodeB>* state) {
    Base::CopyValues(state);
    for (int i = 1; i < state->num_nodes(); ++i) {
      state->node_b(i)->SelfB::set_value(state->node_a(i)->SelfA::value());
    }
  }
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        assert(!a || SelfA::Value(a) == SelfB::Value(b));
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
      int j;
      typename SelfB::Group_::Type value_b;
      if (b->SolidIndex(p, &j)) {
        value_b = SelfB::Group_::Plus(
            b->SelfB::value(), state.corresponding_node_a(p)->SelfA::value());
      } else if (p) {
        value_b = SelfB::Group_::PlusFilter(
            b->SelfB::value(), state.corresponding_node_a(p)->SelfA::value());
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

template<typename Base>
class CommonSetValue : public Base {
 public:
  typedef typename Base::SelfA SelfA;
  typedef typename Base::SelfB SelfB;
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
        if (a) {
          typename SelfB::Group_::Type value =
              state->template NextValue<typename SelfB::Group_::Type>();
          SelfA::SetValue(a, value);
          SelfB::SetValue(b, value);
        }
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
};

template<typename Base>
class CommonAncValue : public Base {
 public:
  typedef typename Base::SelfA SelfA;
  typedef typename Base::SelfB SelfB;
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
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          SelfA::AddToAnc(a, delta);
          SelfB::AddToAnc(b, delta);
        }
        break;
      case 1:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          SelfA::SubtractFromAnc(a, delta);
          SelfB::SubtractFromAnc(b, delta);
        }
        break;
      case 2:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          SelfA::AddToProperAnc(a, delta);
          SelfB::AddToProperAnc(b, delta);
        }
        break;
      case 3:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          SelfA::SubtractFromProperAnc(a, delta);
          SelfB::SubtractFromProperAnc(b, delta);
        }
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
};

template<typename Base>
class CommonDescValue : public Base {
 public:
  typedef typename Base::SelfA SelfA;
  typedef typename Base::SelfB SelfB;
  enum {
    kNumOutcomes = 5,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          for (int i = 1; i < state->num_nodes(); ++i) {
            NodeA* a1 = state->node_a(i);
            if (SameTree(a1, a)) a1->SelfA::add_to_value(delta);
          }
          SelfB::AddToTree(b, delta);
        }
        break;
      case 1:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          for (int i = 1; i < state->num_nodes(); ++i) {
            NodeA* a1 = state->node_a(i);
            if (SameTree(a1, a)) a1->SelfA::subtract_from_value(delta);
          }
          SelfB::SubtractFromTree(b, delta);
        }
        break;
      case 2:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a1 = state->node_a(i);
              if (LeafmostCommonAnc(a1, a) == a) {
                a1->SelfA::add_to_value(delta);
              }
            }
          }
          SelfB::AddToDesc(b, delta);
        }
        break;
      case 3:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a1 = state->node_a(i);
              if (LeafmostCommonAnc(a1, a) == a) {
                a1->SelfA::subtract_from_value(delta);
              }
            }
          }
          SelfB::SubtractFromDesc(b, delta);
        }
        break;
      case 4:
        if (a) {
          typename SelfB::Group_::Type value =
              state->template NextValue<typename SelfB::Group_::Type>();
          SelfB::SetNonDescValue(b, value);
          value = SelfB::Group_::PlusFilter(SelfB::Group_::MinusFilter(value,
                                                                       value),
                                            a->SelfA::value());
          SelfA::SetValue(a, value);
        }
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
};

template<typename Base>
class CommonDescValueWithDesc : public Base {
 public:
  typedef typename Base::SelfA SelfA;
  typedef typename Base::SelfB SelfB;
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
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a1 = state->node_a(i);
              if (a1 != a && LeafmostCommonAnc(a1, a) == a) {
                a1->SelfA::add_to_value(delta);
              }
            }
          }
          SelfB::AddToProperDesc(b, delta);
        }
        break;
      case 1:
        {
          typename SelfB::Group_::Type delta =
              state->template NextDelta<typename SelfB::Group_::Type>();
          if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a1 = state->node_a(i);
              if (a1 != a && LeafmostCommonAnc(a1, a) == a) {
                a1->SelfA::subtract_from_value(delta);
              }
            }
          }
          SelfB::SubtractFromProperDesc(b, delta);
        }
        break;
      default:
        Base::Step(outcome - kNumOutcomes, a, b, state);
    }
  }
};

template<typename Type, typename Base>
class WithStaticValue
    : public CommonSetValue<CommonValue<
                              TypeStaticValue<Type, Base>, Base> > {
};

template<typename Group, typename Base>
class WithAncValue
    : public CommonSetValue<CommonAncValue<
                              CommonValue<
                                TypeAncValue<Group, Base>, Base> > > {
};

template<typename Group, typename Base>
class WithDescValue
    : public CommonDescValue<CommonValue<
                               TypeDescValue<Group, Base>, Base> > {
};

template<typename Group, typename Base>
class WithDescValueWithDesc
    : public CommonDescValueWithDesc<CommonSetValue<
                                       WithDescValue<Group, Base> > > {
};

template<typename Group, typename Base>
class WithAncDescValue
    : public CommonDescValue<CommonAncValue<
                               CommonValue<
                                 TypeAncDescValue<Group, Base>, Base> > > {
};

template<typename Group, typename Base>
class WithAncDescValueWithDesc
    : public CommonDescValueWithDesc<CommonSetValue<
                                       WithAncDescValue<Group, Base> > > {
};

template<typename Semigroup, typename Predicate, typename Base>
class WithAncAggr {
 public:
  typedef naive::WithAncAggr<Semigroup, typename Base::SelfA> SelfA;
  typedef selfadjust::WithAncAggr<Semigroup, typename Base::SelfB> SelfB;
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
  enum {
    kNumOutcomes = 4,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void CopyValues(State<NodeA, NodeB>* state) {
    Base::CopyValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        assert(SelfA::AggrAnc(a) == SelfB::AggrAnc(b));
        break;
      case 1:
        {
          int dir = state->NextDir();
          const Predicate predicate =
              state->template NextPredicate<Predicate>();
          NodeA* a1 = SelfA::FindDirmostAnc(dir, a, predicate);
          NodeB* b1 = SelfB::FindDirmostAnc(dir, b, predicate);
          assert(state->nodes_correspond(a1, b1));
        }
        break;
      case 2:
        assert(SelfA::AggrProperAnc(a) == SelfB::AggrProperAnc(b));
        break;
      case 3:
        {
          int dir = state->NextDir();
          const Predicate predicate =
              state->template NextPredicate<Predicate>();
          NodeA* a1 = SelfA::FindDirmostProperAnc(dir, a, predicate);
          NodeB* b1 = SelfB::FindDirmostProperAnc(dir, b, predicate);
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
      int j;
      if (b->SolidIndex(p, &j)) {
        int k = state.index_b(p);
        aggr[k] = Semigroup::CombineAggrs(aggr[k], aggr[i]);
      }
    }
  }
  static void PrintDecorA(const SelfA* a) { Base::PrintDecorA(a); }
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    fputs(" a", stdout);
    Print(b->delta_aggr());
  }
};

template<typename Semigroup, typename Base>
class PrintDescAggrOfDescValue {
 public:
  typedef selfadjust::WithDescAggrOfDescValue<Semigroup,
                                              typename Base::SelfB> SelfB;
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    fputs(" a", stdout);
    Print(b->delta_aggr());
  }
};

template<typename Semigroup, typename Base>
class PrintAncDescAggr {
 public:
  typedef selfadjust::WithAncDescAggr<Semigroup, typename Base::SelfB> SelfB;
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    fputs(" a", stdout);
    Print(b->delta_aggr());
    fputs(" o", stdout);
    Print(b->delta_other_aggr());
  }
};

template<typename Semigroup,
         typename Predicate,
         typename Printer,
         typename Base>
class CommonDescAggr {
 private:
  typedef typename Base::SelfA SelfA;
  typedef typename Printer::SelfB SelfB;

 public:
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
  enum {
    kNumOutcomes = 6,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void CopyValues(State<NodeA, NodeB>* state) {
    Base::CopyValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        {
          typename Semigroup::Type aggr_a = Semigroup::empty_aggr();
          for (int i = 1; i < state->num_nodes(); ++i) {
            NodeA* a1 = state->node_a(i);
            if (SameTree(a1, a)) {
              aggr_a = Semigroup::CombineAggrs(
                  aggr_a,
                  Semigroup::AggrFromValue(a1->SelfA::value()));
            }
          }
          assert(aggr_a == SelfB::AggrTree(b));
        }
        break;
      case 1:
        {
          int dir = state->NextDir();
          const Predicate predicate =
              state->template NextPredicate<Predicate>();
          if (NodeB* b1 = SelfB::FindDirmostTree(dir, b, predicate)) {
            NodeA* a1 = state->corresponding_node_a(b1);
            assert(SameTree(a1, a));
            assert(predicate(Semigroup::AggrFromValue(a1->SelfA::value())));
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a2 = state->node_a(i);
              if (a2 == a1 ||
                  LeafmostCommonAnc(a2, a1) !=
                  (dir == selfadjust::kLeaf ? a1 : a2)) {
                continue;
              }
              assert(!predicate(Semigroup::AggrFromValue(a2->SelfA::value())));
            }
          } else if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a2 = state->node_a(i);
              assert(!SameTree(a2, a) ||
                     !predicate(Semigroup::AggrFromValue(a2->SelfA::value())));
            }
          }
        }
        break;
      case 2:
        {
          typename Semigroup::Type aggr_a = Semigroup::empty_aggr();
          if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a1 = state->node_a(i);
              if (LeafmostCommonAnc(a1, a) == a) {
                aggr_a = Semigroup::CombineAggrs(
                    aggr_a,
                    Semigroup::AggrFromValue(a1->SelfA::value()));
              }
            }
          }
          assert(aggr_a == SelfB::AggrDesc(b));
        }
        break;
      case 3:
        {
          int dir = state->NextDir();
          const Predicate predicate =
              state->template NextPredicate<Predicate>();
          if (NodeB* b1 = SelfB::FindDirmostDesc(dir, b, predicate)) {
            NodeA* a1 = state->corresponding_node_a(b1);
            assert(a);
            assert(LeafmostCommonAnc(a1, a) == a);
            assert(predicate(Semigroup::AggrFromValue(a1->SelfA::value())));
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a2 = state->node_a(i);
              if (a2 == a1 ||
                  LeafmostCommonAnc(a2, a) != a ||
                  LeafmostCommonAnc(a2, a1) !=
                  (dir == selfadjust::kLeaf ? a1 : a2)) {
                continue;
              }
              assert(!predicate(Semigroup::AggrFromValue(a2->SelfA::value())));
            }
          } else if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a2 = state->node_a(i);
              assert(LeafmostCommonAnc(a2, a) != a ||
                     !predicate(Semigroup::AggrFromValue(a2->SelfA::value())));
            }
          }
        }
        break;
      case 4:
        {
          typename Semigroup::Type aggr_a = Semigroup::empty_aggr();
          if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a1 = state->node_a(i);
              if (a1 != a && LeafmostCommonAnc(a1, a) == a) {
                aggr_a = Semigroup::CombineAggrs(
                    aggr_a,
                    Semigroup::AggrFromValue(a1->SelfA::value()));
              }
            }
          }
          assert(aggr_a == SelfB::AggrProperDesc(b));
        }
        break;
      case 5:
        {
          int dir = state->NextDir();
          const Predicate predicate =
              state->template NextPredicate<Predicate>();
          if (NodeB* b1 = SelfB::FindDirmostProperDesc(dir, b, predicate)) {
            NodeA* a1 = state->corresponding_node_a(b1);
            assert(a);
            assert(a1 != a);
            assert(LeafmostCommonAnc(a1, a) == a);
            assert(predicate(Semigroup::AggrFromValue(a1->SelfA::value())));
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a2 = state->node_a(i);
              if (a2 == a1 ||
                  a2 == a ||
                  LeafmostCommonAnc(a2, a) != a ||
                  LeafmostCommonAnc(a2, a1) !=
                  (dir == selfadjust::kLeaf ? a1 : a2)) {
                continue;
              }
              assert(!predicate(Semigroup::AggrFromValue(a2->SelfA::value())));
            }
          } else if (a) {
            for (int i = 1; i < state->num_nodes(); ++i) {
              NodeA* a2 = state->node_a(i);
              assert(a2 == a ||
                     LeafmostCommonAnc(a2, a) != a ||
                     !predicate(Semigroup::AggrFromValue(a2->SelfA::value())));
            }
          }
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
  static void PrintDecorA(const SelfA* a) { Base::PrintDecorA(a); }
  static void PrintDecorB(const SelfB* b) {
    Base::PrintDecorB(b);
    Printer::PrintDecorB(b);
  }
};

template<typename Semigroup, typename Predicate, typename Base>
class WithDescAggrOfDescValue
    : public CommonDescAggr<Semigroup,
                            Predicate,
                            PrintDescAggrOfDescValue<Semigroup, Base>,
                            Base> {
 public:
  typedef typename Base::SelfA SelfA;
  typedef typename PrintDescAggrOfDescValue<Semigroup, Base>::SelfB SelfB;
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
};

template<typename Semigroup, typename Predicate, typename Base>
class WithAncDescAggr
    : public CommonDescAggr<Semigroup,
                            Predicate,
                            PrintAncDescAggr<Semigroup, Base>,
                            Base> {
 public:
  typedef typename Base::SelfA SelfA;
  typedef typename PrintAncDescAggr<Semigroup, Base>::SelfB SelfB;
  template<typename NodeA, typename NodeB>
  static void ValidateAggrs(const State<NodeA, NodeB>& state,
                            const std::vector<const NodeB*>& top_order) {
    Base::ValidateAggrs(state, top_order);
    std::vector<typename Semigroup::Type> aggr(state.num_nodes());
    for (int i = 1; i < state.num_nodes(); ++i) {
      aggr[i] = Semigroup::AggrFromValue(state.node_a(i)->SelfA::value());
    }
    std::vector<typename Semigroup::Type> aggr1(state.num_nodes(),
                                                Semigroup::empty_aggr());
    std::vector<typename Semigroup::Type> other_aggr(state.num_nodes(),
                                                     Semigroup::empty_aggr());
    for (typename std::vector<const NodeB*>::const_iterator it =
             top_order.begin();
         it != top_order.end();
         ++it) {
      const NodeB* b = *it;
      const NodeB* p = b->parent();
      if (!p) continue;
      int j;
      int i = state.index_b(b);
      int k = state.index_b(p);
      if (b->SolidIndex(p, &j)) {
        assert(aggr[i] ==
               SelfB::Group_::Plus(b->SelfB::delta_aggr(),
                                   state.node_a(i)->SelfA::value()));
        aggr[k] = Semigroup::CombineAggrs(aggr[k], aggr[i]);
      } else {
        assert(Semigroup::CombineAggrs(aggr[i], aggr1[i]) ==
               SelfB::Group_::PlusFilter(b->SelfB::delta_aggr(),
                                         state.node_a(i)->SelfA::value()));
        other_aggr[i] = Semigroup::CombineAggrs(other_aggr[i], aggr[i]);
      }
      if (!b->DottedIndex(p, &j)) {
        aggr1[k] = Semigroup::CombineAggrs(aggr1[k], other_aggr[i]);
      }
      assert(other_aggr[i] ==
             SelfB::Group_::PlusFilter(b->SelfB::delta_other_aggr(),
                                       state.node_a(i)->SelfA::value()));
      other_aggr[k] = Semigroup::CombineAggrs(other_aggr[k], other_aggr[i]);
    }
  }
};

template<typename Base>
class WithEvertBy {
 public:
  typedef naive::WithEvertBy<typename Base::SelfA> SelfA;
  typedef selfadjust::WithEvertBy<typename Base::SelfB> SelfB;
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
  enum {
    kNumOutcomes = 1,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void CopyValues(State<NodeA, NodeB>* state) {
    Base::CopyValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        SelfA::Evert(a);
        SelfB::Evert(b);
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
class WithEvert {
 public:
  typedef naive::WithEvert<typename Base::SelfA> SelfA;
  typedef selfadjust::WithEvert<typename Base::SelfB> SelfB;
  typedef typename Base::VisitorA VisitorA;
  typedef typename Base::VisitorB VisitorB;
  enum {
    kNumOutcomes = 1,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  template<typename NodeA, typename NodeB>
  static void CopyValues(State<NodeA, NodeB>* state) {
    Base::CopyValues(state);
  }
  template<typename NodeA, typename NodeB>
  static void Step(int outcome,
                   NodeA* a,
                   NodeB* b,
                   State<NodeA, NodeB>* state) {
    switch (outcome) {
      case 0:
        SelfA::Evert(a);
        SelfB::Evert(b);
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

enum Color { kWhite, kGray, kBlack };

template<typename Base>
class EndTree {
 public:
  typedef naive::EndTree<typename Base::SelfA> NodeA;
  typedef selfadjust::EndTree<typename Base::SelfB> NodeB;
  enum {
    kNumOutcomes = 9,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  explicit EndTree(int num_nodes);
  void AssembleByTable(int parent[]);
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
  DISALLOW_COPY_AND_ASSIGN(EndTree);
};

template<typename Base>
EndTree<Base>::EndTree(int num_nodes) : state_(num_nodes) {
}

template<typename Base>
void EndTree<Base>::AssembleByTable(int parent[]) {
  Base::CopyValues(&state_);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    state_.node_a(i)->set_parent(state_.node_a(parent[i]));
    state_.node_b(i)->set_parent(state_.node_b(parent[i]));
  }
  Assemble(state_.node_a(1), state_.num_nodes() - 1);
  Assemble(state_.node_b(1), state_.num_nodes() - 1);
}

template<typename Base>
void EndTree<Base>::Step() {
  int outcome = state_.NextOutcome(kTotalOutcomes);
  NodeA* a;
  NodeB* b;
  state_.NextCorrespondingNodes(&a, &b);
  switch (outcome) {
    case 0:
      assert(state_.nodes_correspond(Root(a), Root(b)));
      break;
    case 1:
      {
        NodeA* a2 = Parent(a);
        NodeB* b2 = state_.corresponding_node_b(a2);
        NodeA* a1 = Cut(a);
        NodeB* b1 = Cut(b);
        assert(LeafmostCommonAnc(a2, a1) == a1);
        assert(LeafmostCommonAnc(b2, b1) == b1);
        break;
      }
    case 2:
    case 3:
      {
        NodeA* a1;
        NodeB* b1;
        state_.NextCorrespondingNodes(&a1, &b1);
        bool connected_a = SameTree(a, a1);
        bool connected_b = SameTree(b, b1);
        assert(connected_a == connected_b);
        assert(connected_a ||
               state_.nodes_correspond(Link(a, a1), Link(b, b1)));
      }
      break;
    case 4:
      assert(state_.nodes_correspond(Parent(a), Parent(b)));
      break;
    case 5:
      {
        NodeA* a1;
        NodeB* b1;
        state_.NextCorrespondingNodes(&a1, &b1);
        assert(state_.nodes_correspond(LeafmostCommonAnc(a, a1),
                                       LeafmostCommonAnc(b, b1)));
      }
      break;
    case 6:
      {
        typename Base::VisitorA va;
        EvertByTraversing(a, &va);
        typename Base::VisitorB vb;
        EvertByTraversing(b, &vb);
      }
      break;
    case 7:
      Base::CopyValues(&state_);
      for (int i = 1; i < state_.num_nodes(); ++i) {
        NodeB* b = state_.node_b(i);
        b->set_parent(state_.corresponding_node_b(
            state_.corresponding_node_a(b)->parent()));
      }
      Assemble(state_.node_b(1), state_.num_nodes() - 1);
      break;
    case 8:
      if (state_.NextOutcome(32)) break;
      for (int i = 1; i < state_.num_nodes(); ++i) {
        CutOneOfMany(state_.node_a(i));
        CutOneOfMany(state_.node_b(i));
      }
      break;
    default:
      Base::Step(outcome - kNumOutcomes, a, b, &state_);
  }
}

template<typename Base>
void EndTree<Base>::Validate() const {
  AcyclicA();
  AcyclicB();
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    const NodeB* p = b->parent();
    int j;
    if (!b->SolidIndex(p, &j)) CompareTopology(b, 0, p);
  }
  Base::CompareValues(state_);
  std::vector<const NodeB*> top_order;
  state_.OrderNodesTopologically(&top_order);
  Base::ValidateAggrs(state_, top_order);
}

template<typename Base>
void EndTree<Base>::AcyclicA() const {
  std::vector<Color> color(state_.num_nodes(), kWhite);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeA* a = state_.node_a(i);
    do {
      int j = state_.index_a(a);
      if (color[j] == kBlack) break;
      assert(color[j] == kWhite);
      color[j] = kGray;
    } while ((a = a->parent()));
    a = state_.node_a(i);
    do {
      int j = state_.index_a(a);
      if (color[j] == kBlack) break;
      assert(color[j] == kGray);
      color[j] = kBlack;
    } while ((a = a->parent()));
  }
  for (int i = 1; i < state_.num_nodes(); ++i) assert(color[i] == kBlack);
}

template<typename Base>
void EndTree<Base>::AcyclicB() const {
  std::vector<bool> visited(state_.num_nodes(), false);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    const NodeB* p = b->parent();
    int j;
    if (!b->SolidIndex(p, &j)) TraverseB(b, p, &visited);
  }
  for (int i = 1; i < state_.num_nodes(); ++i) assert(visited[i]);
}

template<typename Base>
void EndTree<Base>::TraverseB(const NodeB* b,
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
const typename EndTree<Base>::NodeB* EndTree<Base>::CompareTopology(
    const NodeB* b,
    int flipped,
    const NodeB* r) const {
  if (!b) return r;
  flipped ^= b->SolidFlipped();
  const NodeB* l =
      CompareTopology(b->solid(selfadjust::kRoot ^ flipped), flipped, r);
  assert(state_.nodes_correspond(state_.corresponding_node_a(b)->parent(), l));
  return CompareTopology(b->solid(selfadjust::kLeaf ^ flipped), flipped, b);
}

template<typename Base>
void EndTree<Base>::PrintA() const {
  fputs("<<<\n", stdout);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeA* a = state_.node_a(i);
    printf("  %d:", i);
    Base::PrintDecorA(a);
    if (a->parent()) printf(" -> %d", state_.index_a(a->parent()));
    putchar('\n');
  }
  fputs(">>>\n", stdout);
}

template<typename Base>
void EndTree<Base>::PrintB() const {
  bool soft_space = false;
  fputs("<<<\n", stdout);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    const NodeB* p = b->parent();
    int j;
    if (b->SolidIndex(p, &j)) continue;
    if (soft_space) putchar('\n');
    if (p) printf(" %d\n", state_.index_b(p));
    PrintConcreteTree(b, 3, p ? '\\' : '@', p);
    soft_space = true;
  }
  fputs(">>>\n", stdout);
}

template<typename Base>
void EndTree<Base>::PrintConcreteTree(const NodeB* b,
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

////////////////////////////////////////

template<typename Base>
class EndTreeWithDesc {
 public:
  typedef naive::EndTree<typename Base::SelfA> NodeA;
  typedef selfadjust::EndTreeWithDesc<typename Base::SelfB> NodeB;
  enum {
    kNumOutcomes = 12,
    kTotalOutcomes = kNumOutcomes + Base::kTotalOutcomes
  };
  explicit EndTreeWithDesc(int num_nodes);
  void AssembleByTable(int parent[]);
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
                         const char* relationship,
                         const NodeB* p) const;
  State<NodeA, NodeB> state_;
  DISALLOW_COPY_AND_ASSIGN(EndTreeWithDesc);
};

template<typename Base>
EndTreeWithDesc<Base>::EndTreeWithDesc(int num_nodes) : state_(num_nodes) {
}

template<typename Base>
void EndTreeWithDesc<Base>::AssembleByTable(int parent[]) {
  Base::CopyValues(&state_);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    state_.node_a(i)->set_parent(state_.node_a(parent[i]));
    state_.node_b(i)->set_parent(state_.node_b(parent[i]));
  }
  Assemble(state_.node_a(1), state_.num_nodes() - 1);
  Assemble(state_.node_b(1), state_.num_nodes() - 1);
}

template<typename Base>
void EndTreeWithDesc<Base>::Step() {
  int outcome = state_.NextOutcome(kTotalOutcomes);
  NodeA* a;
  NodeB* b;
  state_.NextCorrespondingNodes(&a, &b);
  switch (outcome) {
    case 0:
      assert(state_.nodes_correspond(Root(a), Root(b)));
      break;
    case 1:
      {
        NodeA* a2 = Parent(a);
        NodeB* b2 = state_.corresponding_node_b(a2);
        NodeA* a1 = Cut(a);
        NodeB* b1 = Cut(b);
        assert(LeafmostCommonAnc(a2, a1) == a1);
        assert(LeafmostCommonAnc(b2, b1) == b1);
        break;
      }
    case 2:
    case 3:
      {
        NodeA* a1;
        NodeB* b1;
        state_.NextCorrespondingNodes(&a1, &b1);
        bool connected_a = SameTree(a, a1);
        bool connected_b = SameTree(b, b1);
        assert(connected_a == connected_b);
        assert(connected_a ||
               state_.nodes_correspond(Link(a, a1), Link(b, b1)));
      }
      break;
    case 4:
      assert(state_.nodes_correspond(Parent(a), Parent(b)));
      break;
    case 5:
      {
        NodeA* a1;
        NodeB* b1;
        state_.NextCorrespondingNodes(&a1, &b1);
        assert(state_.nodes_correspond(LeafmostCommonAnc(a, a1),
                                       LeafmostCommonAnc(b, b1)));
      }
      break;
    case 6:
      {
        typename Base::VisitorA va;
        EvertByTraversing(a, &va);
        typename Base::VisitorB vb;
        EvertByTraversing(b, &vb);
      }
      break;
    case 7:
      Base::CopyValues(&state_);
      for (int i = 1; i < state_.num_nodes(); ++i) {
        NodeB* b = state_.node_b(i);
        b->set_parent(state_.corresponding_node_b(
            state_.corresponding_node_a(b)->parent()));
      }
      Assemble(state_.node_b(1), state_.num_nodes() - 1);
      break;
    case 8:
      if (state_.NextOutcome(32)) break;
      for (int i = 1; i < state_.num_nodes(); ++i) {
        CutOneOfMany(state_.node_a(i));
        CutOneOfMany(state_.node_b(i));
      }
      break;
    case 9:
      {
        NodeA* pa = Parent(a);
        int dir = state_.NextDir();
        NodeB* pb = ContractDirward(dir, b);
        assert(state_.nodes_correspond(pa, pb));
        if (!pa) break;
        NodeA* v;
        NodeA* w;
        if (dir == naive::kLeaf) {
          v = a;
          w = pa;
          if (w) v->set_parent(w->parent());
        } else if (dir == naive::kRoot) {
          v = pa;
          w = a;
        } else {
          assert(false);
        }
        w->set_parent(NULL);
        for (int i = 1; i < state_.num_nodes(); ++i) {
          NodeA* a1 = state_.node_a(i);
          if (a1->parent() == w) a1->set_parent(v);
        }
      }
      break;
    case 10:
      if (a) {
        NodeB* b1 = Leaf(b);
        assert(LeafmostCommonAnc(b1, b) == b);
        for (int i = 1; i < state_.num_nodes(); ++i) {
          assert(Parent(state_.node_b(i)) != b1);
        }
      } else {
        assert(!Leaf(b));
      }
      break;
    case 11:
      if (a) {
        if (NodeB* b1 = Child(b)) {
          assert(Parent(b1) == b);
        } else {
          for (int i = 1; i < state_.num_nodes(); ++i) {
            assert(Parent(state_.node_b(i)) != b);
          }
        }
      } else {
        assert(!Child(b));
      }
      break;
    default:
      Base::Step(outcome - kNumOutcomes, a, b, &state_);
  }
}

template<typename Base>
void EndTreeWithDesc<Base>::Validate() const {
  AcyclicA();
  AcyclicB();
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    const NodeB* p = b->parent();
    int j;
    if (b->SolidIndex(p, &j)) continue;
    if (p) {
      const NodeB* b1 = b;
      while (p->dashed() != b1) {
        b1 = p;
        p = p->parent();
        assert(p);
      }
    }
    CompareTopology(b, 0, p);
  }
  Base::CompareValues(state_);
  std::vector<const NodeB*> top_order;
  state_.OrderNodesTopologically(&top_order);
  Base::ValidateAggrs(state_, top_order);
}

template<typename Base>
void EndTreeWithDesc<Base>::AcyclicA() const {
  std::vector<Color> color(state_.num_nodes(), kWhite);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeA* a = state_.node_a(i);
    do {
      int j = state_.index_a(a);
      if (color[j] == kBlack) break;
      assert(color[j] == kWhite);
      color[j] = kGray;
    } while ((a = a->parent()));
    a = state_.node_a(i);
    do {
      int j = state_.index_a(a);
      if (color[j] == kBlack) break;
      assert(color[j] == kGray);
      color[j] = kBlack;
    } while ((a = a->parent()));
  }
  for (int i = 1; i < state_.num_nodes(); ++i) assert(color[i] == kBlack);
}

template<typename Base>
void EndTreeWithDesc<Base>::AcyclicB() const {
  std::vector<bool> visited(state_.num_nodes(), false);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    if (!b->parent()) TraverseB(b, NULL, &visited);
  }
  for (int i = 1; i < state_.num_nodes(); ++i) assert(visited[i]);
}

template<typename Base>
void EndTreeWithDesc<Base>::TraverseB(const NodeB* b,
                                      const NodeB* p,
                                      std::vector<bool>* visited) const {
  if (!b) return;
  int j = state_.index_b(b);
  assert(!(*visited)[j]);
  (*visited)[j] = true;
  assert(b->parent() == p);
  TraverseB(b->dashed(), b, visited);
  TraverseB(b->solid(0), b, visited);
  TraverseB(b->solid(1), b, visited);
  TraverseB(b->dotted(0), b, visited);
  TraverseB(b->dotted(1), b, visited);
}

template<typename Base>
const typename EndTreeWithDesc<Base>::NodeB*
EndTreeWithDesc<Base>::CompareTopology(const NodeB* b,
                                       int flipped,
                                       const NodeB* r) const {
  if (!b) return r;
  flipped ^= b->SolidFlipped();
  const NodeB* l =
      CompareTopology(b->solid(selfadjust::kRoot ^ flipped), flipped, r);
  assert(state_.nodes_correspond(state_.corresponding_node_a(b)->parent(), l));
  return CompareTopology(b->solid(selfadjust::kLeaf ^ flipped), flipped, b);
}

template<typename Base>
void EndTreeWithDesc<Base>::PrintA() const {
  fputs("<<<\n", stdout);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeA* a = state_.node_a(i);
    printf("  %d:", i);
    Base::PrintDecorA(a);
    if (a->parent()) printf(" -> %d", state_.index_a(a->parent()));
    putchar('\n');
  }
  fputs(">>>\n", stdout);
}

template<typename Base>
void EndTreeWithDesc<Base>::PrintB() const {
  bool soft_space = false;
  fputs("<<<\n", stdout);
  for (int i = 1; i < state_.num_nodes(); ++i) {
    const NodeB* b = state_.node_b(i);
    if (b->parent()) continue;
    if (soft_space) putchar('\n');
    PrintConcreteTree(b, 3, "@", NULL);
    soft_space = true;
  }
  fputs(">>>\n", stdout);
}

template<typename Base>
void EndTreeWithDesc<Base>::PrintConcreteTree(const NodeB* b,
                                              int indent,
                                              const char* relationship,
                                              const NodeB* p) const {
  if (!b) return;
  if (indent > 40) {
    printf("%*s...\n", indent, relationship);
    return;
  }
  PrintConcreteTree(b->dotted(1), indent + 2, ".'", b);
  PrintConcreteTree(b->solid(1), indent + 2, "/", b);
  printf("%*s%d%c", indent, relationship, state_.index_b(b),
         b->parent() == p ? ':' : '#');
  Base::PrintDecorB(b);
  putchar('\n');
  PrintConcreteTree(b->dashed(), indent + 2, "->", b);
  PrintConcreteTree(b->solid(0), indent + 2, "\\", b);
  PrintConcreteTree(b->dotted(0), indent + 2, "'.", b);
}

static const int kNumNodes = 8;

template<typename Test>
void TestAssembleHelper(int id[], int n, int p, Test* test) {
  static int parent[kNumNodes];
  int u = id[n];
  parent[u] = p;
  if (n == kNumNodes - 1) {
    test->AssembleByTable(parent);
    test->Validate();
    return;
  }
  int i;
  for (i = 0; i <= n; ++i) TestAssembleHelper(id, n + 1, id[i], test);
  for (; i < kNumNodes; ++i) {
    int v = id[i];
    id[i] = u;
    id[n] = v;
    parent[v] = p;
    TestAssembleHelper(id, n + 1, v, test);
    id[i] = v;
  }
  id[n] = u;
}

template<typename Test>
void TestAssemble() {
  int id[kNumNodes];
  for (int i = 0; i < kNumNodes; ++i) id[i] = i;
  Test test(kNumNodes);
  TestAssembleHelper(id, 1, 0, &test);
}
}  // namespace

int main(int /*argc*/, char** argv) {
  static char buf[BUFSIZ];
  setvbuf(stdout, buf, _IOLBF, sizeof buf);
  TestAssemble<DTREE_CONFIG>();
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
