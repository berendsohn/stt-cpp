// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include <assert.h>
#include <stdlib.h>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <stack>
#include <utility>

#include "example/flow/data.h"
#include "example/flow/error.h"

namespace flow {

namespace {

struct Graph {
  Graph();
  // the vector into which the elements of end point
  std::vector<int> succ;
  // the successors of node u are stored from succ_end[u - 1] to
  // succ_end[u]
  std::vector<std::vector<int>::iterator> succ_end;
};

Graph::Graph() : succ(), succ_end() {
}

template<typename ForwardPredicate, typename BackwardPredicate>
void GraphFromSolution(const Network& network,
                       const Solution& solution,
                       const ForwardPredicate& forward,
                       const BackwardPredicate& backward,
                       Graph* graph) {
  graph->succ.clear();
  graph->succ_end.clear();
  graph->succ_end.resize(network.n + 1);
  // compute the outdegree of each node
  std::vector<int> outdeg(network.n + 1, 0);
  std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
  for (std::vector<int>::const_iterator j = solution.flows.begin();
       j != solution.flows.end();
       ++i, ++j) {
    if (forward(i->cap, *j)) ++outdeg[i->tail];
    if (backward(i->cap, *j)) ++outdeg[i->head];
  }
  // compute the number of arcs
  int m = 0;
  for (std::vector<int>::const_iterator j = outdeg.begin();
       j != outdeg.end();
       ++j) {
    m += *j;
  }
  graph->succ.resize(m);
  // fill graph->succ
  std::vector<int>::iterator begin = graph->succ.begin();
  std::vector<int>::const_iterator k = outdeg.begin();
  for (std::vector<std::vector<int>::iterator>::iterator j =
           graph->succ_end.begin();
       j != graph->succ_end.end();
       ++k, ++j) {
    *j = begin;
    begin += *k;
  }
  i = network.arcs.begin();
  for (std::vector<int>::const_iterator j = solution.flows.begin();
       j != solution.flows.end();
       ++i, ++j) {
    if (forward(i->cap, *j)) *graph->succ_end[i->tail]++ = i->head;
    if (backward(i->cap, *j)) *graph->succ_end[i->head]++ = i->tail;
  }
}

struct ForwardResidualCapacity : public std::binary_function<int, int, bool> {
  bool operator()(int c, int f) const { return f < c; }
};

struct BackwardResidualCapacity : public std::binary_function<int, int, bool> {
  bool operator()(int /*c*/, int f) const { return f > 0; }
};

// Constructs the transpose of the residual network.
void ResidualTransposeFromSolution(const Network& network,
                                   const Solution& solution,
                                   Graph* graph) {
  GraphFromSolution(network, solution, BackwardResidualCapacity(),
                    ForwardResidualCapacity(), graph);
}
}  // namespace

void CutFromSolution(const Network& network,
                     const Solution& solution,
                     std::vector<bool>* cut) {
  cut->clear();
  cut->resize(network.n + 1, false);
  Graph graph;
  ResidualTransposeFromSolution(network, solution, &graph);
  // depth-first search
  int u = network.t;
  (*cut)[u] = true;
  std::stack<int> st;
  while (true) {
    for (std::vector<int>::const_iterator i = graph.succ_end[u - 1];
         i != graph.succ_end[u];
         ++i) {
      if ((*cut)[*i]) continue;
      (*cut)[*i] = true;
      st.push(*i);
    }
    if (st.empty()) break;
    u = st.top();
    st.pop();
  }
}

namespace {

class Total {
 public:
  Total();
  void Add(int x);
  void Subtract(int x);
  bool Equals(int x) const;
  bool IsNonnegative() const;
  bool IsNonnegativeInt() const;
  bool IsPositive() const;
  unsigned int low() const { return low_; }
  int high() const { return high_; }
 private:
  // unsigned for defined overflow
  unsigned int low_;
  int high_;
  // copy and assign for std::vector
};

Total::Total() : low_(0), high_(0) {
}

void Total::Add(int x) {
  assert(x >= 0);
  unsigned int sum = low() + x;
  if (sum < low()) ++high_;
  low_ = sum;
}

void Total::Subtract(int x) {
  assert(x >= 0);
  unsigned int difference = low() - x;
  if (difference > low()) --high_;
  low_ = difference;
}

bool Total::Equals(int x) const {
  assert(x >= 0);
  return low() == static_cast<unsigned int>(x) && high() == 0;
}

bool Total::IsNonnegative() const {
  return high() >= 0;
}

bool Total::IsNonnegativeInt() const {
  return (high() == 0 &&
          low() <= static_cast<unsigned int>(std::numeric_limits<int>::max()));
}

bool Total::IsPositive() const {
  return high() > 0 || (high() == 0 && low() > 0);
}

void FlowExcessesFromSolution(const Network& network,
                              const Solution& solution,
                              std::vector<Total>* excess) {
  excess->clear();
  excess->resize(network.n + 1);
  std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
  for (std::vector<int>::const_iterator j = solution.flows.begin();
       j != solution.flows.end();
       ++i, ++j) {
    (*excess)[i->tail].Subtract(*j);
    (*excess)[i->head].Add(*j);
  }
}

class ErrorTracker {
 public:
  explicit ErrorTracker(const char* path);
  virtual ~ErrorTracker();
  void IncompleteError();
  void Error(const char* format, ...) __attribute__((format(printf, 2, 3)));
  void Format(const char* format, ...) __attribute__((format(printf, 2, 3)));
 private:
  ErrorWriter ew_;
  bool error_;
  DISALLOW_COPY_AND_ASSIGN(ErrorTracker);
};

ErrorTracker::ErrorTracker(const char* path)
    : ew_(path, stderr), error_(false) {
}

ErrorTracker::~ErrorTracker() {
  if (error_) exit(EXIT_FAILURE);
}

void ErrorTracker::IncompleteError() {
  ew_.Error(0, 0, kError);
  error_ = true;
}

void ErrorTracker::Error(const char* format, ...) {
  IncompleteError();
  va_list ap;
  va_start(ap, format);
  ew_.FormatArgs(format, ap);
  va_end(ap);
  ew_.Format("\n");
}

void ErrorTracker::Format(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  ew_.FormatArgs(format, ap);
  va_end(ap);
}

void VerifyFlowExcesses(const Network& network,
                        SolutionKind kind,
                        const Solution& solution,
                        const std::vector<Total>& excess,
                        ErrorTracker* et) {
  for (int u = 1; u <= network.n; ++u) {
    if (u == network.s || u == network.t) continue;
    if (!excess[u].IsNonnegative()) {
      et->Error("the flow excess at node %d is negative", u);
    } else if (kind == kFlow && excess[u].IsPositive()) {
      et->Error("the flow excess at node %d is positive", u);
    }
  }
  // validate the objective value
  const Total& sink_excess = excess[network.t];
  if (!sink_excess.Equals(solution.value)) {
    if (sink_excess.high() < 0) {
      et->Error("the flow excess at the sink is negative");
    } else if (sink_excess.IsNonnegativeInt()) {
      et->Error(
          "the flow excess at the sink is %u, but the objective value is %d",
          sink_excess.low(), solution.value);
    } else {
      et->Error("the flow excess at the sink is too large for an int");
    }
  }
}

void VerifyOptimality(const Network& network,
                      const Solution& solution,
                      const std::vector<Total>& excess,
                      ErrorTracker* et) {
  Graph graph;
  ResidualTransposeFromSolution(network, solution, &graph);
  // search breadth-first for an augmenting path
  std::vector<int> parent(network.n + 1, 0);
  int u = network.t;
  parent[u] = u;
  std::queue<int> q;
  while (true) {
    for (std::vector<int>::const_iterator i = graph.succ_end[u - 1];
         i != graph.succ_end[u];
         ++i) {
      if (parent[*i] != 0) continue;
      parent[*i] = u;
      q.push(*i);
    }
    if (q.empty()) break;
    u = q.front();
    q.pop();
  }
  for (u = 1; u <= network.n; ++u) {
    if (u == network.t ||
        !(u == network.s || excess[u].IsPositive()) ||
        parent[u] == 0) {
      continue;
    }
    et->IncompleteError();
    et->Format("augmenting path:");
    while (true) {
      et->Format(" %d", u);
      if (u == network.t) break;
      u = parent[u];
    }
    et->Format("\n");
    break;
  }
}

struct ForwardFlow : public std::binary_function<int, int, bool> {
  bool operator()(int /*c*/, int f) const { return f > 0; }
};

struct BackwardFlow : public std::binary_function<int, int, bool> {
  bool operator()(int /*c*/, int /*f*/) const { return false; }
};

enum Color { kWhite, kGray, kBlack };

void VerifyAcyclicity(const Network& network,
                      const Solution& solution,
                      ErrorTracker* et) {
  // construct the transpose
  Graph graph;
  GraphFromSolution(network, solution, BackwardFlow(), ForwardFlow(), &graph);
  // search depth-first for a cycle
  std::stack<int> st;
  for (int u = 1; u <= network.n; ++u) st.push(u);
  std::vector<Color> color(network.n + 1, kWhite);
  std::vector<int> parent(network.n + 1);
  while (!st.empty()) {
    int u = st.top();
    if (color[u] == kWhite) {
      color[u] = kGray;
      for (std::vector<int>::const_iterator i = graph.succ_end[u - 1];
           i != graph.succ_end[u];
           ++i) {
        if (color[*i] == kWhite) {
          st.push(*i);
          parent[*i] = u;
        } else if (color[*i] == kGray) {
          et->IncompleteError();
          et->Format("flow cycle:");
          int v = u;
          while (true) {
            et->Format(" %d", v);
            if (v == *i) break;
            v = parent[v];
          }
          et->Format(" %d\n", u);
        } else if (color[*i] == kBlack) {
        } else {
          assert(false);
        }
      }
    } else if (color[u] == kGray) {
      st.pop();
      color[u] = kBlack;
    } else if (color[u] == kBlack) {
      st.pop();
    } else {
      assert(false);
    }
  }
}
}  // namespace

void VerifyCapacityBoundsOrDie(const char* path, const Network& network) {
  ErrorTracker et(path);
  std::vector<Total> incoming_cap(network.n + 1);
  std::vector<Total> outgoing_cap(network.n + 1);
  typedef std::map<std::pair<int, int>, int> ArcMap;
  ArcMap arc_map_cap;
  for (std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
       i != network.arcs.end();
       ++i) {
    incoming_cap[i->head].Add(i->cap);
    outgoing_cap[i->tail].Add(i->cap);
    ArcMap::const_iterator j =
        arc_map_cap.find(std::make_pair(i->head, i->tail));
    if (j == arc_map_cap.end()) {
      arc_map_cap[std::make_pair(i->tail, i->head)] = i->cap;
    } else if (i->cap > std::numeric_limits<int>::max() - j->second) {
      et.Error("the total capacity on edge %d %d is too large for an int",
               i->tail, i->head);
    }
  }
  for (int u = 1; u <= network.n; ++u) {
    if (!incoming_cap[u].IsNonnegativeInt()) {
      et.Error(
          "the total incoming capacity at node %d is too large for an int", u);
    }
    if (!outgoing_cap[u].IsNonnegativeInt()) {
      et.Error(
          "the total outgoing capacity at node %d is too large for an int", u);
    }
  }
}

void VerifySolutionOrDie(const char* path,
                         const Network& network,
                         SolutionKind kind,
                         const Solution& solution) {
  network.Validate();
  solution.Validate(network);
  std::vector<Total> excess;
  FlowExcessesFromSolution(network, solution, &excess);
  ErrorTracker et(path);
  VerifyFlowExcesses(network, kind, solution, excess, &et);
  VerifyOptimality(network, solution, excess, &et);
  if (kind == kFlow) VerifyAcyclicity(network, solution, &et);
}
}  // namespace flow
