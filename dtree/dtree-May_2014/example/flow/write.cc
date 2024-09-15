// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include "example/flow/data.h"

namespace flow {

void WriteNetwork(const Network& network, FILE* stream) {
  fprintf(stream, "p max %d %d\nn %d s\nn %d t\n", network.n,
          static_cast<int>(network.arcs.size()), network.s, network.t);
  for (std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
       i != network.arcs.end();
       ++i) {
    fprintf(stream, "a %d %d %d\n", i->tail, i->head, i->cap);
  }
}

void WriteSolution(const Network& network,
                   const Solution& solution,
                   FILE* stream) {
  fprintf(stream, "s %d\n", solution.value);
  std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
  for (std::vector<int>::const_iterator j = solution.flows.begin();
       j != solution.flows.end();
       ++i, ++j) {
    fprintf(stream, "f %d %d %d\n", i->tail, i->head, *j);
  }
}
}  // namespace flow
