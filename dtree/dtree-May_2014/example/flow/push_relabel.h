// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef EXAMPLE_FLOW_PUSH_RELABEL_H_
#define EXAMPLE_FLOW_PUSH_RELABEL_H_

#include "example/flow/data.h"

namespace flow {

// Cherkassky and Goldberg,
//   "On implementing push-relabel method for the maximum flow problem"
// Goldberg and Tarjan, "A new approach to the maximum-flow problem"

// push_relabel.cc
// Computes a maximum preflow via the push-relabel algorithm with
// highest-label discharges and global relabeling and gap relabeling.
void MaxPreflowPR(const Network& network, Solution* solution);

// send_relabel.cc
// Computes a maximum preflow via the "send-relabel" algorithm
// (push-relabel with Sleator--Tarjan trees) with highest-label
// discharges and global relabeling and gap relabeling.
void MaxPreflowSR(const Network& network, Solution* solution);
}  // namespace flow
#endif  // EXAMPLE_FLOW_PUSH_RELABEL_H_
