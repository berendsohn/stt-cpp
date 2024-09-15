// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

// Generates pseudorandom spanning trees of a grid graph and displays
// them as mazes.  Uses the Markov chain proved by Broder to be
// rapidly mixing.
//
// Broder, "Generating random spanning trees"

#include <stdio.h>

#include "dtree/tree.h"
#include "dtree/tree-inl.h"
#include "util/random.h"

namespace {

typedef dtree::WithAncAggr<dtree::Count<int>, dtree::Begin<> > Count;
typedef dtree::WithEvert<Count> E;
typedef dtree::EndTree<E> Node;

class Maze {
 public:
  Maze();
  void Step(util::Random* r);
  void Print(FILE* stream);

 private:
  enum {
    kHeight = 11,
    kWidth = 35,
    kNumHorizontalEdges = kHeight * (kWidth - 1),
    kNumVerticalEdges = (kHeight - 1) * kWidth,
    kNumEdges = kNumHorizontalEdges + kNumVerticalEdges,
    kEmpty = ' ',
    kFilled = '#'
  };
  Node* node(int i, int j) {
    assert(0 <= i);
    assert(i < kHeight);
    assert(0 <= j);
    assert(j < kWidth);
    return &node_[i][j];
  }
  static void PrintFirstOrLastRow(FILE* stream);
  void PrintEvenRow(int i, FILE* stream);
  void PrintOddRow(int i, FILE* stream);
  void PrintEdge(int i1, int j1, int i2, int j2, FILE* stream);
  Node node_[kHeight][kWidth];
  Maze(const Maze&);
  void operator=(const Maze&);
};

Maze::Maze() : node_() {
  for (int j = 1; j < kWidth; ++j) Link(node(0, j), node(0, j - 1));
  for (int i = 1; i < kHeight; ++i) {
    for (int j = 0; j < kWidth; ++j) Link(node(i, j), node(i - 1, j));
  }
}

void Maze::Step(util::Random* r) {
  // sample an edge {u, v} not in the tree uniformly at random
  Node* u;
  Node* v;
  do {
    int k = r->Next<int>(kNumEdges);
    int i1;
    int j1;
    int i2;
    int j2;
    if (k < kNumHorizontalEdges) {
      i1 = k / (kWidth - 1);
      j1 = k % (kWidth - 1);
      i2 = i1;
      j2 = j1 + 1;
    } else {
      k -= kNumHorizontalEdges;
      i1 = k / kWidth;
      j1 = k % kWidth;
      i2 = i1 + 1;
      j2 = j1;
    }
    u = node(i1, j1);
    E::Evert(u);
    v = node(i2, j2);
  } while (Parent(v) == u);
  // sample an edge uniformly at random from the fundamental cycle
  int k = r->Next(Count::AggrAnc(v));
  if (k == 0) return;
  Node* w = Count::FindRootmostAnc(v, dtree::Index(k));
  assert(w != u);
  Cut(w);
  Link(u, v);
}

void Maze::Print(FILE* stream) {
  PrintFirstOrLastRow(stream);
  PrintEvenRow(0, stream);
  for (int i = 1; i < kHeight; ++i) {
    PrintOddRow(i, stream);
    PrintEvenRow(i, stream);
  }
  PrintFirstOrLastRow(stream);
}

void Maze::PrintFirstOrLastRow(FILE* stream) {
  for (int k = 0; k < kWidth * 2 + 1; ++k) putc(kFilled, stream);
  putc('\n', stream);
}

void Maze::PrintEvenRow(int i, FILE* stream) {
  putc(kFilled, stream);
  putc(kEmpty, stream);
  for (int j = 1; j < kWidth; ++j) {
    PrintEdge(i, j - 1, i, j, stream);
    putc(kEmpty, stream);
  }
  putc(kFilled, stream);
  putc('\n', stream);
}

void Maze::PrintOddRow(int i, FILE* stream) {
  putc(kFilled, stream);
  for (int j = 0; j < kWidth; ++j) {
    PrintEdge(i - 1, j, i, j, stream);
    putc(kFilled, stream);
  }
  putc('\n', stream);
}

void Maze::PrintEdge(int i1, int j1, int i2, int j2, FILE* stream) {
  Node* u = node(i1, j1);
  E::Evert(u);
  putc(Parent(node(i2, j2)) == u ? kEmpty : kFilled, stream);
}

void Usage() __attribute__((noreturn));

void Usage() {
  fputs("usage: maze [seed]\n", stderr);
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
  if (argc != 1 && argc != 2) Usage();
  util::Random r;
  if (argc == 2) r.set_seed(Scan(argv[1]));
  Maze maze;
  while (true) {
    for (int i = 0; i < 1000000; ++i) maze.Step(&r);
    putc('\n', stdout);
    maze.Print(stdout);
  }
}
