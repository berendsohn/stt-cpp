// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

// Writes n pseudorandom points in the unit square to stdout.  Can be
// piped to tsp.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits>

#include "util/random.h"

namespace {

void Usage() __attribute__((noreturn));

void Usage() {
  fputs("usage: random_points n [seed]\n", stderr);
  exit(EXIT_FAILURE);
}

uint64_t Scan(const char* nptr) {
  char* endptr;
  if (*nptr == '\0') Usage();
  uint64_t n = strtoull(nptr, &endptr, 10);
  if (*endptr != '\0') Usage();
  return n;
}
}  // namespace

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) Usage();
  uint64_t n = Scan(argv[1]);
  util::Random r;
  if (argc == 3) r.set_seed(Scan(argv[2]));
  while (n-- > 0) {
    printf("%.*g\t%.*g\n", std::numeric_limits<double>::digits10,
           ldexp(r.Next(), -64), std::numeric_limits<double>::digits10,
           ldexp(r.Next(), -64));
  }
}
