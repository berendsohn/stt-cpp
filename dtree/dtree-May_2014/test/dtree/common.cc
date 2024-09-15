// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

namespace {

template<typename Base>
struct FreePair {
  FreePair() : desc(), anc() {}
  FreePair(Base d, Base a) : desc(d), anc(a) {}
  Base desc;
  Base anc;
};

struct FreeValue1 {
  FreeValue1() : head(), tail() {}
  FreeValue1(uint64_t h, uint64_t t) : head(h), tail(t) {}
  uint64_t head;
  uint64_t tail;
};

typedef FreePair<FreeValue1> FreeValue;

struct FreeAggr1 {
  FreeAggr1() : tail() {}
  explicit FreeAggr1(uint64_t t) : tail(t) {}
  uint64_t tail;
};

typedef FreePair<FreeAggr1> FreeAggr;

class FreeGroup {
 public:
  typedef FreeValue Type;
  template<typename BaseX, typename BaseY>
  static FreePair<BaseX> Plus(FreePair<BaseX> x, FreePair<BaseY> y) {
    return FreePair<BaseX>(Plus(x.desc, y.desc), Plus(x.anc, y.anc));
  }
  template<typename BaseX, typename BaseY>
  static FreePair<BaseX> Minus(FreePair<BaseX> x, FreePair<BaseY> y) {
    return FreePair<BaseX>(Minus(x.desc, y.desc), Minus(x.anc, y.anc));
  }
  template<typename BaseX, typename BaseY>
  static FreePair<BaseX> PlusFilter(FreePair<BaseX> x, FreePair<BaseY> y) {
    return FreePair<BaseX>(x.desc, Plus(x.anc, y.anc));
  }
  template<typename BaseX, typename BaseY>
  static FreePair<BaseX> MinusFilter(FreePair<BaseX> x, FreePair<BaseY> y) {
    return FreePair<BaseX>(x.desc, Minus(x.anc, y.anc));
  }

 private:
  static FreeValue1 Plus(FreeValue1 x, FreeValue1 y) {
    if (x.head == x.tail) return y;
    if (y.head == y.tail) return x;
    assert(x.tail == y.head);
    return FreeValue1(x.head, y.tail);
  }
  static FreeAggr1 Plus(FreeAggr1 x, FreeValue1 y) {
    if (!x.tail) return FreeAggr1();
    if (y.head == y.tail) return x;
    assert(x.tail == y.head);
    return FreeAggr1(y.tail);
  }
  static FreeValue1 Minus(FreeValue1 y) { return FreeValue1(y.tail, y.head); }
  static FreeValue1 Minus(FreeValue1 x, FreeValue1 y) {
    return Plus(x, Minus(y));
  }
  static FreeAggr1 Minus(FreeAggr1 x, FreeValue1 y) {
    return Plus(x, Minus(y));
  }
};

class FreeSemigroup {
 public:
  typedef FreeAggr Type;
  static FreeAggr AggrFromValue(FreeValue x) {
    return FreeAggr(AggrFromValue(x.desc), AggrFromValue(x.anc));
  }
  static FreeAggr CombineAggrs(FreeAggr x, FreeAggr y) {
    return FreeAggr(CombineAggrs(x.desc, y.desc), CombineAggrs(x.anc, y.anc));
  }
  static FreeAggr empty_aggr() { return FreeAggr(); }
 private:
  static FreeAggr1 AggrFromValue(FreeValue1 x) { return FreeAggr1(x.tail); }
  static FreeAggr1 CombineAggrs(FreeAggr1 x, FreeAggr1 y) {
    if (!x.tail) return y;
    assert(!y.tail || x.tail == y.tail);
    return x;
  }
};

template<typename CountType, typename SumType>
inline bool operator==(dtree::CasAggr<CountType, SumType> x,
                       dtree::CasAggr<CountType, SumType> y) {
  return x.count == y.count && x.sum == y.sum;
}

template<typename Base>
inline bool operator==(dtree::DpAggr<Base> x, dtree::DpAggr<Base> y) {
  return x.lw() == y.lw() && x.rw() == y.rw();
}

template<typename Base>
inline bool operator==(dtree::DpValue<Base> x, dtree::DpValue<Base> y) {
  return x.lw() == y.lw() && x.rw() == y.rw() && x.flipped() == y.flipped();
}

inline bool operator==(FreeValue1 x, FreeValue1 y) {
  return x.head == y.head && x.tail == y.tail;
}

inline bool operator==(FreeAggr1 x, FreeAggr1 y) {
  return x.tail == y.tail;
}

template<typename Base>
inline bool operator==(FreePair<Base> x, FreePair<Base> y) {
  return x.desc == y.desc && x.anc == y.anc;
}

template<typename Type>
void Print(Type x);

template<>
inline void Print(int x) {
  printf("%d", x);
}

template<>
inline void Print(dtree::CasAggr<int, int> x) {
  printf("C%dS%d", x.count, x.sum);
}

template<>
inline void Print(dtree::DpAggr<int> x) {
  printf("L%dR%d", x.lw(), x.rw());
}

template<>
inline void Print(dtree::DpValue<int> x) {
  printf("L%dR%dF%d", x.lw(), x.rw(), x.flipped());
}

template<>
inline void Print(double x) {
  printf("%.*g", std::numeric_limits<double>::digits10, x);
}

template<>
inline void Print(dtree::CasAggr<double, double> x) {
  printf("C%.*gS%.*g", std::numeric_limits<double>::digits10, x.count,
         std::numeric_limits<double>::digits10, x.sum);
}

template<>
inline void Print(dtree::DpAggr<double> x) {
  printf("L%.*gR%.*g", std::numeric_limits<double>::digits10, x.lw(),
         std::numeric_limits<double>::digits10, x.rw());
}

template<>
inline void Print(dtree::DpValue<double> x) {
  printf("L%.*gR%.*gF%d", std::numeric_limits<double>::digits10, x.lw(),
         std::numeric_limits<double>::digits10, x.rw(), x.flipped());
}

inline int low_order_bits(uint64_t n) {
  return static_cast<int>(n & 0xfff);
}

template<>
inline void Print(FreeValue1 x) {
  printf("%03x,%03x", low_order_bits(x.head), low_order_bits(x.tail));
}

template<>
inline void Print(FreePair<FreeValue1> x) {
  Print(x.desc);
  putchar(',');
  Print(x.anc);
}

template<>
inline void Print(FreeAggr1 x) {
  printf("%03x", low_order_bits(x.tail));
}

template<>
inline void Print(FreePair<FreeAggr1> x) {
  Print(x.desc);
  putchar(',');
  Print(x.anc);
}

template<typename Type>
Type NextValue(util::Random* source);

template<>
inline int NextValue<int>(util::Random* source) {
  return source->Next(8);
}

template<>
inline dtree::DpValue<int> NextValue<dtree::DpValue<int> >(
    util::Random* source) {
  dtree::DpValue<int> x;
  x.set_lw(source->Next(16) - 8);
  x.set_rw(source->Next(16) - 8);
  x.set_flipped(source->Next(2));
  return x;
}

template<>
inline double NextValue<double>(util::Random* source) {
  return source->Next(8);
}

template<>
inline dtree::DpValue<double> NextValue<dtree::DpValue<double> >(
    util::Random* source) {
  dtree::DpValue<double> x;
  x.set_lw(source->Next(16) - 8);
  x.set_rw(source->Next(16) - 8);
  x.set_flipped(source->Next(2));
  return x;
}

template<>
inline FreeValue NextValue<FreeValue>(util::Random* source) {
  FreeValue1 desc(source->Next(), 0);
  FreeValue1 anc(source->Next(), 0);
  return FreeValue(desc, anc);
}

template<typename Type>
inline Type NextDelta(util::Random* source) {
  return NextValue<Type>(source);
}

template<>
inline dtree::DpValue<int> NextDelta<dtree::DpValue<int> >(
    util::Random* source) {
  dtree::DpValue<int> x = NextValue<dtree::DpValue<int> >(source);
  x.set_flipped(0);
  return x;
}

template<>
inline dtree::DpValue<double> NextDelta<dtree::DpValue<double> >(
    util::Random* source) {
  dtree::DpValue<double> x = NextValue<dtree::DpValue<double> >(source);
  x.set_flipped(0);
  return x;
}

template<>
inline FreeValue NextDelta<FreeValue>(util::Random* /*source*/) {
  return FreeValue();
}

class NullPredicate_ {
 public:
  NullPredicate_() {}
  template<typename Type>
  bool operator()(Type /*x*/) const { return false; }
};

template<typename Predicate>
Predicate NextPredicate(util::Random* source);

template<>
inline dtree::Less_<int> NextPredicate<dtree::Less_<int> >(
    util::Random* source) {
  return dtree::Less_<int>(source->Next(32) - 16);
}

template<>
inline dtree::LessEqual_<int> NextPredicate<dtree::LessEqual_<int> >(
    util::Random* source) {
  return dtree::LessEqual_<int>(source->Next(32) - 16);
}

template<>
inline dtree::Greater_<int> NextPredicate<dtree::Greater_<int> >(
    util::Random* source) {
  return dtree::Greater_<int>(source->Next(32) - 16);
}

template<>
inline dtree::GreaterEqual_<int> NextPredicate<dtree::GreaterEqual_<int> >(
    util::Random* source) {
  return dtree::GreaterEqual_<int>(source->Next(32) - 16);
}

template<>
inline dtree::Index_<int> NextPredicate<dtree::Index_<int> >(
    util::Random* source) {
  return dtree::Index_<int>(source->Next(32));
}

template<>
inline dtree::IndexByCount_<int> NextPredicate<dtree::IndexByCount_<int> >(
    util::Random* source) {
  return dtree::IndexByCount_<int>(source->Next(32));
}

template<>
inline dtree::Nonzero_ NextPredicate<dtree::Nonzero_>(
    util::Random* /*source*/) {
  return dtree::Nonzero_();
}

template<>
inline dtree::NonzeroAnd_<int>
NextPredicate<dtree::NonzeroAnd_<int> >(util::Random* source) {
  return dtree::NonzeroAnd_<int>(source->Next(8));
}

template<>
inline dtree::DwLess_<int> NextPredicate<dtree::DwLess_<int> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwLess_<int>(dir, source->Next(32) - 16);
}

template<>
inline dtree::DwLessEqual_<int> NextPredicate<dtree::DwLessEqual_<int> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwLessEqual_<int>(dir, source->Next(32) - 16);
}

template<>
inline dtree::DwGreater_<int> NextPredicate<dtree::DwGreater_<int> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwGreater_<int>(dir, source->Next(32) - 16);
}

template<>
inline dtree::DwGreaterEqual_<int> NextPredicate<dtree::DwGreaterEqual_<int> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwGreaterEqual_<int>(dir, source->Next(32) - 16);
}

template<>
inline dtree::Less_<double> NextPredicate<dtree::Less_<double> >(
    util::Random* source) {
  return dtree::Less_<double>(source->Next(32) - 16);
}

template<>
inline dtree::LessEqual_<double> NextPredicate<dtree::LessEqual_<double> >(
    util::Random* source) {
  return dtree::LessEqual_<double>(source->Next(32) - 16);
}

template<>
inline dtree::Greater_<double> NextPredicate<dtree::Greater_<double> >(
    util::Random* source) {
  return dtree::Greater_<double>(source->Next(32) - 16);
}

template<>
inline dtree::GreaterEqual_<double>
NextPredicate<dtree::GreaterEqual_<double> >(util::Random* source) {
  return dtree::GreaterEqual_<double>(source->Next(32) - 16);
}

template<>
inline dtree::Index_<double> NextPredicate<dtree::Index_<double> >(
    util::Random* source) {
  return dtree::Index_<double>(source->Next(32));
}

template<>
inline dtree::IndexByCount_<double>
NextPredicate<dtree::IndexByCount_<double> >(
    util::Random* source) {
  return dtree::IndexByCount_<double>(source->Next(32));
}

template<>
inline dtree::DwLess_<double> NextPredicate<dtree::DwLess_<double> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwLess_<double>(dir, source->Next(32) - 16);
}

template<>
inline dtree::DwLessEqual_<double> NextPredicate<dtree::DwLessEqual_<double> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwLessEqual_<double>(dir, source->Next(32) - 16);
}

template<>
inline dtree::DwGreater_<double> NextPredicate<dtree::DwGreater_<double> >(
    util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwGreater_<double>(dir, source->Next(32) - 16);
}

template<>
inline dtree::DwGreaterEqual_<double>
NextPredicate<dtree::DwGreaterEqual_<double> >(util::Random* source) {
  int dir = source->Next(2);
  return dtree::DwGreaterEqual_<double>(dir, source->Next(32) - 16);
}

template<>
inline NullPredicate_ NextPredicate<NullPredicate_>(util::Random* /*source*/) {
  return NullPredicate_();
}

template<typename Node>
class Forwarder {
 public:
  Node* operator()(Node* lhs, const Node* rhs, Node* ptr) {
    return lhs + (ptr - rhs);
  }
};

template<typename Node>
void Reallocate(util::ScopedArray<Node>* a, int num_nodes) {
  util::ScopedArray<Node> b(new Node[num_nodes]);
  for (int i = 0; i < num_nodes; ++i) {
    b[i].CopyFrom((*a)[i], Forwarder<Node>());
  }
  a->swap(b);
}

template<typename NodeA, typename NodeB>
class State {
 public:
  explicit State(int num_nodes);
  int num_nodes() const { return num_nodes_; }
  NodeA* node_a(int i) { return i ? &base_a_[i] : NULL; }
  const NodeA* node_a(int i) const { return i ? &base_a_[i] : NULL; }
  NodeB* node_b(int i) { return i ? &base_b_[i] : NULL; }
  const NodeB* node_b(int i) const { return i ? &base_b_[i] : NULL; }
  int index_a(const NodeA* a) const { return a ? a - base_a_.get() : 0; }
  int index_b(const NodeB* b) const { return b ? b - base_b_.get() : 0; }
  NodeA* corresponding_node_a(NodeB* b) {
    return b ? &base_a_[b - base_b_.get()] : NULL;
  }
  const NodeA* corresponding_node_a(const NodeB* b) const {
    return b ? &base_a_[b - base_b_.get()] : NULL;
  }
  NodeB* corresponding_node_b(NodeA* a) {
    return a ? &base_b_[a - base_a_.get()] : NULL;
  }
  const NodeB* corresponding_node_b(const NodeB* a) const {
    return a ? &base_b_[a - base_a_.get()] : NULL;
  }
  bool nodes_correspond(const NodeA* a, const NodeB* b) const {
    return index_a(a) == index_b(b);
  }
  int NextOutcome(int num_outcomes) { return source_.Next(num_outcomes); }
  void NextCorrespondingNodes(NodeA** a, NodeB** b) {
    int i = source_.Next(num_nodes());
    *a = node_a(i);
    *b = node_b(i);
  }
  int NextDir() { return source_.Next(2); }
  template<typename Type>
  Type NextValue() { return ::NextValue<Type>(&source_); }
  template<typename Type>
  Type NextDelta() { return ::NextDelta<Type>(&source_); }
  template<typename Predicate>
  Predicate NextPredicate() { return ::NextPredicate<Predicate>(&source_); }
  void OrderNodesTopologically(std::vector<const NodeB*>* top_order) const;
  void ReallocateAB();

 private:
  util::Random source_;
  int num_nodes_;
  util::ScopedArray<NodeA> base_a_;
  util::ScopedArray<NodeB> base_b_;
  DISALLOW_COPY_AND_ASSIGN(State);
};

template<typename NodeA, typename NodeB>
State<NodeA, NodeB>::State(int num_nodes)
    : source_(),
      num_nodes_(num_nodes),
      base_a_(new NodeA[num_nodes]),
      base_b_(new NodeB[num_nodes]) {
}

template<typename NodeA, typename NodeB>
void State<NodeA, NodeB>::OrderNodesTopologically(
    std::vector<const NodeB*>* top_order) const {
  std::vector<bool> visited(num_nodes(), false);
  top_order->resize(num_nodes() - 1);
  typename std::vector<const NodeB*>::reverse_iterator it =
      top_order->rbegin();
  visited[0] = true;
  for (int i = 1; i < num_nodes(); ++i) {
    const NodeB* b = node_b(i);
    typename std::vector<const NodeB*>::reverse_iterator prev_it = it;
    while (true) {
      int j = index_b(b);
      if (visited[j]) break;
      visited[j] = true;
      *it++ = b;
      b = b->parent();
    }
    reverse(prev_it, it);
  }
}

template<typename NodeA, typename NodeB>
void State<NodeA, NodeB>::ReallocateAB() {
  Reallocate(&base_a_, num_nodes());
  Reallocate(&base_b_, num_nodes());
}

class Begin {
 public:
  typedef naive::Begin<> SelfA;
  typedef selfadjust::Begin<> SelfB;
  class VisitorA {
   public:
    VisitorA() {}
    template<typename Node>
    void operator()(Node* /*node*/) {}
   private:
    DISALLOW_COPY_AND_ASSIGN(VisitorA);
  };
  class VisitorB {
   public:
    VisitorB() {}
    template<typename Node>
    void operator()(Node* /*node*/) {}
   private:
    DISALLOW_COPY_AND_ASSIGN(VisitorB);
  };
  enum { kTotalOutcomes = 0 };
  template<typename NodeA, typename NodeB>
  static void CopyValues(State<NodeA, NodeB>* /*state*/) {}
  template<typename NodeA, typename NodeB>
  static void Step(int /*outcome*/,
                   NodeA* /*a*/,
                   NodeB* /*b*/,
                   State<NodeA, NodeB>* /*state*/) {
    assert(false);
  }
  template<typename NodeA, typename NodeB>
  static void CompareValues(const State<NodeA, NodeB>& /*state*/) {}
  template<typename NodeA, typename NodeB>
  static void ValidateAggrs(const State<NodeA, NodeB>& /*state*/,
                            const std::vector<const NodeB*>& /*top_order*/) {
  }
  static void PrintDecorA(const SelfA* /*a*/) {}
  static void PrintDecorB(const SelfB* /*b*/) {}
};
}  // namespace
