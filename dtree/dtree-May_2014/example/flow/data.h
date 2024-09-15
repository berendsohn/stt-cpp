// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef EXAMPLE_FLOW_DATA_H_
#define EXAMPLE_FLOW_DATA_H_

#include <stdio.h>
#include <vector>

#include "util/disallow_copy_and_assign.h"

namespace flow {

struct ArcDescriptor {
  ArcDescriptor();
  ArcDescriptor(int tail, int head, int cap);
  void Validate(int n) const;
  void Reset();
  int tail;
  int head;
  int cap;
};

struct Network {
  Network();
  void Validate() const;
  void Reset();
  // number of nodes in the network
  // nodes are 1..n
  int n;
  // the source node
  int s;
  // the sink node
  int t;
  std::vector<ArcDescriptor> arcs;
};

struct Solution {
  Solution();
  // Initializes this solution to the zero flow on network.
  void InitializeFrom(const Network& network);
  // Asserts that value >= 0 and 0 <= flows[i] <= network.arcs[i].cap,
  // which are the checks performed by Reader::ReadSolutionOrDie.  Use
  // VerifySolutionOrDie to verify that a solution is a maximum flow.
  void Validate(const Network& network) const;
  void Reset();
  // the objective value
  int value;
  // flows[i] corresponds to network.arcs[i]
  std::vector<int> flows;
};

// read.cc
class Reader {
 public:
  Reader(const char* path, FILE* stream);
  virtual ~Reader();
  // Reads a network in DIMACS format.
  void ReadNetworkOrDie(Network* network);
  // Reads a solution in DIMACS format.
  void ReadSolutionOrDie(const Network& network, Solution* solution);
 private:
  class ReaderImpl;
  ReaderImpl* impl_;
  DISALLOW_COPY_AND_ASSIGN(Reader);
};

// verify.cc
// Computes the set of nodes that can reach the sink node t in the
// residual network.  The indicator function of this set is stored in
// cut as an (n + 1)-element boolean vector indexed logically by node.
void CutFromSolution(const Network& network,
                     const Solution& solution,
                     std::vector<bool>* cut);
// Verifies that, for each node, the total incoming capacity and the
// total outgoing capacity can be represented by the type int.  Also
// verifies that, for each pair of nodes, the total capacity of arcs
// from one node to the other can be represented similarly.
//
// The purpose of these checks is that node excesses and residual
// capacities on *darts* can be represented by the type int.
void VerifyCapacityBoundsOrDie(const char* path, const Network& network);
// Verifies that solution is a maximum preflow for network.  If kind
// == kFlow, verifies additionally that solution satisfies
// conservation of flow and is acyclic.
enum SolutionKind { kPreflow, kFlow };
void VerifySolutionOrDie(const char* path,
                         const Network& network,
                         SolutionKind kind,
                         const Solution& solution);

// write.cc
// Writes a network in DIMACS format.
void WriteNetwork(const Network& network, FILE* stream);
// Writes a solution in DIMACS format.
void WriteSolution(const Network& network,
                   const Solution& solution,
                   FILE* stream);
}  // namespace flow
#endif  // EXAMPLE_FLOW_DATA_H_
