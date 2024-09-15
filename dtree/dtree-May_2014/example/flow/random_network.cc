// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

// Writes a pseudorandom network to stdout.  Can be piped to flow.

#include <inttypes.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <utility>

#include "example/flow/data.h"
#include "util/random.h"

namespace {

void Usage() __attribute__((noreturn));

void Usage() {
  fputs("usage: random_network n m cap_max [seed]\n", stderr);
  exit(EXIT_FAILURE);
}

uint64_t Scan(const char* nptr) {
  char* endptr;
  if (*nptr == '\0') Usage();
  uint64_t n = strtoull(nptr, &endptr, 10);
  if (*endptr != '\0') Usage();
  return n;
}

void Check(const char* descr, uint64_t n, uint64_t min, uint64_t max) {
  if (n < min || max < n) {
    fprintf(stderr,
            "random_network: %s: integer between %"PRIu64" and %"PRIu64
            " expected\n",
            descr, min, max);
    exit(EXIT_FAILURE);
  }
}
}  // namespace

int main(int argc, char** argv) {
  if (argc != 4 && argc != 5) Usage();
  uint64_t n = Scan(argv[1]);
  uint64_t m = Scan(argv[2]);
  uint64_t cap_max = Scan(argv[3]);
  util::Random r;
  if (argc == 5) r.set_seed(Scan(argv[4]));
  Check("n", n, 2, INT32_MAX - 1);
  uint64_t m_max = n * (n - 1);
  Check("m", m, 0, std::min(m_max, static_cast<uint64_t>(INT32_MAX)));
  Check("cap_max", cap_max, 0, INT32_MAX);
  flow::Network network;
  network.n = n;
  network.s = 1;
  network.t = n;
  network.arcs.resize(m);
  std::vector<flow::ArcDescriptor>::iterator i = network.arcs.begin();
  typedef std::map<uint64_t, uint64_t> Deck;
  Deck deck;
  for (uint64_t j = 0; j < m; ++i, ++j) {
    uint64_t k = j + r.Next(m_max - j);
    Deck::iterator k_item = deck.insert(std::make_pair(k, k)).first;
    Deck::iterator j_item = deck.find(j);
    uint64_t card = j_item != deck.end() ? j_item->second : j;
    std::swap(k_item->second, card);
    int32_t tail = card % n;
    int32_t head = card / n;
    if (tail <= head) ++head;
    i->tail = tail + 1;
    i->head = head + 1;
    i->cap = r.Next(cap_max + 1);
  }
  WriteNetwork(network, stdout);
}
