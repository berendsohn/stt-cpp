// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include "example/flow/push_relabel.h"

#include <algorithm>
#include <map>
#include <utility>

#include "util/scoped.h"

namespace flow {

namespace {

// intrusive doubly-linked list
struct Element {
  Element();
  Element* prev;
  Element* next;
};

Element::Element() : prev(this), next(this) {
}

inline void Remove(Element* u) {
  Element* u_next = u->next;
  u->prev->next = u_next;
  u_next->prev = u->prev;
}

inline void Insert(Element* b, Element* u) {
  b->prev->next = u;
  u->prev = b->prev;
  u->next = b;
  b->prev = u;
}

inline bool Empty(Element* b) {
  return b->next == b;
}

struct Dart;

struct Node : public Element {
  Node();
  Dart* last;
  Dart* current;
  int label;
  int excess;
};

Node::Node() : last(NULL), current(NULL), label(0), excess(0) {
}

struct Dart {
  Dart();
  Node* head;
  // reverse dart
  Dart* rev;
  // residual capacity
  int cap;
};

Dart::Dart() : head(NULL), rev(NULL), cap(0) {
}

class State {
 public:
  State();
  void Activate(Node* u);
  void Deactivate(Node* u);
  void SetLabel(Node* u, int label);
  void InitializeQueue();
  void Discharge(Node* u);
  void Run(const Network& network, Solution* solution);

 private:
  typedef std::map<std::pair<int, int>, Dart*> DartMap;
  DartMap dart_map_;
  util::ScopedArray<Node> node_;
  util::ScopedArray<Dart> dart_;
  int n_;
  Node* s_;
  Node* t_;
  // buckets indexed by label
  util::ScopedArray<Element> active_;
  util::ScopedArray<Element> inactive_;
  // upper bound on labels of active nodes
  int upper_active_;
  // upper bound on labels of all nodes
  int upper_all_;
  DISALLOW_COPY_AND_ASSIGN(State);
};

inline State::State()
    : dart_map_(),
      node_(NULL),
      dart_(NULL),
      n_(0),
      s_(NULL),
      t_(NULL),
      active_(NULL),
      inactive_(NULL),
      upper_active_(0),
      upper_all_(0) {
}

inline void State::Activate(Node* u) {
  Remove(u);
  Insert(&active_[u->label], u);
  if (u->label != n_ && u->label > upper_active_) {
    upper_active_ = u->label;
    if (u->label > upper_all_) upper_all_ = u->label;
  }
}

inline void State::Deactivate(Node* u) {
  Remove(u);
  Insert(&inactive_[u->label], u);
}

inline void State::SetLabel(Node* u, int label) {
  u->label = label;
  Activate(u);
}

inline void State::InitializeQueue() {
  for (int i = 0; i <= n_; ++i) {
    Element* b = &active_[i];
    Remove(b);
    b->next = b->prev = b;
  }
  for (int i = 1; i <= n_; ++i) {
    Node* u = &node_[i];
    u->current = u[-1].last;
    u->label = n_;
  }
  upper_active_ = 0;
  SetLabel(t_, 0);
  for (int i = 0; true; ++i) {
    Element* b = &active_[i];
    Element* e = b->next;
    if (e == b) break;
    do {
      Node* u = static_cast<Node*>(e);
      e = u->next;
      u->label = i;
      if (u->excess == 0) Deactivate(u);
      for (Dart* a = u[-1].last; a != u->last; ++a) {
        if (a->rev->cap == 0) continue;
        Node* v = a->head;
        if (v->label == n_) SetLabel(v, i + 1);
      }
    } while (e != b);
  }
}

inline void State::Discharge(Node* u) {
  Dart* a = u->current;
  do {
    // push from u
    if (a->cap == 0) continue;
    Node* v = a->head;
    if (u->label <= v->label) continue;
    bool no_excess = u->excess <= a->cap;
    int delta = no_excess ? u->excess : a->cap;
    u->excess -= delta;
    a->cap -= delta;
    a->rev->cap += delta;
    if (v->excess == 0) Activate(v);
    v->excess += delta;
    if (no_excess) {
      u->current = a;
      Deactivate(u);
      return;
    }
  } while (++a != u->last);
  u->current = u[-1].last;
  int old_label = u->label;
  // relabel u
  int low = n_ - 1;
  for (a = u[-1].last; a != u->last; ++a) {
    if (a->cap > 0) low = std::min(low, a->head->label);
  }
  SetLabel(u, low + 1);
  // gap heuristic
  if (Empty(&active_[old_label]) && Empty(&inactive_[old_label])) {
    for (int i = old_label + 1; i <= upper_all_; ++i) {
      for (Element* e = inactive_[i].next; e != &inactive_[i]; e = e->next) {
        static_cast<Node*>(e)->label = n_;
      }
    }
    upper_all_ = old_label - 1;
  }
}

inline void State::Run(const Network& network, Solution* solution) {
  solution->InitializeFrom(network);
  // initialize the internal network representation
  for (std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
       i != network.arcs.end();
       ++i) {
    dart_map_[std::make_pair(i->tail, i->head)] = NULL;
    dart_map_[std::make_pair(i->head, i->tail)] = NULL;
  }
  node_.reset(new Node[network.n + 1]);
  dart_.reset(new Dart[dart_map_.size()]);
  int u = 0;
  Dart* last = dart_.get();
  for (DartMap::iterator i = dart_map_.begin(); i != dart_map_.end(); ++i) {
    for (; u < i->first.first; ++u) node_[u].last = last;
    i->second = last++;
  }
  for (; u <= network.n; ++u) node_[u].last = last;
  for (std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
       i != network.arcs.end();
       ++i) {
    Dart* a = dart_map_[std::make_pair(i->tail, i->head)];
    Dart* b = dart_map_[std::make_pair(i->head, i->tail)];
    a->head = &node_[i->head];
    a->rev = b;
    a->cap = i->cap;
    b->head = &node_[i->tail];
    b->rev = a;
  }
  n_ = network.n;
  s_ = &node_[network.s];
  t_ = &node_[network.t];
  // push from s
  for (Dart* a = s_[-1].last; a != s_->last; ++a) {
    a->head->excess += a->cap;
    a->rev->cap += a->cap;
    a->cap = 0;
  }
  // initialize the queue
  active_.reset(new Element[n_ + 1]);
  inactive_.reset(new Element[n_ + 1]);
  InitializeQueue();
  // discharge repeatedly
  int steps = n_;
  while (upper_active_ > 0) {
    Element* b = &active_[upper_active_];
    Element* e = b->next;
    if (e == b) {
      --upper_active_;
      continue;
    }
    Node* u = static_cast<Node*>(e);
    Discharge(u);
    if (--steps == 0) {
      InitializeQueue();
      steps = n_;
    }
  }
  // extract the solution
  solution->value = t_->excess;
  std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
  for (std::vector<int>::iterator j = solution->flows.begin();
       j != solution->flows.end();
       ++i, ++j) {
    *j = std::max(i->cap - dart_map_[std::make_pair(i->tail, i->head)]->cap,
                  0);
  }
}
}  // namespace

void MaxPreflowPR(const Network& network, Solution* solution) {
  State state;
  state.Run(network, solution);
}
}  // namespace flow
