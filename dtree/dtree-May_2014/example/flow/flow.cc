// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

// Computes and verifies preflows.

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "example/flow/push_relabel.h"

namespace {

void ComputePreflow(
    const char* path,
    FILE* stream,
    void (*compute)(const flow::Network& network, flow::Solution* solution)) {
  flow::Reader reader(path, stream);
  flow::Network network;
  reader.ReadNetworkOrDie(&network);
  VerifyCapacityBoundsOrDie(path, network);
  flow::Solution solution;
  compute(network, &solution);
  WriteNetwork(network, stdout);
  WriteSolution(network, solution, stdout);
}

void ComputePR(const char* path, FILE* stream) {
  ComputePreflow(path, stream, flow::MaxPreflowPR);
}

void ComputeSR(const char* path, FILE* stream) {
  ComputePreflow(path, stream, flow::MaxPreflowSR);
}

void VerifyCapacityBounds(const char* path, FILE* stream) {
  flow::Reader reader(path, stream);
  flow::Network network;
  reader.ReadNetworkOrDie(&network);
  VerifyCapacityBoundsOrDie(path, network);
}

void VerifySolution(const char* path, FILE* stream, flow::SolutionKind kind) {
  flow::Reader reader(path, stream);
  flow::Network network;
  reader.ReadNetworkOrDie(&network);
  flow::Solution solution;
  reader.ReadSolutionOrDie(network, &solution);
  VerifySolutionOrDie(path, network, kind, solution);
  // verify the cut just to be sure
  std::vector<bool> cut;
  CutFromSolution(network, solution, &cut);
  int value = 0;
  for (std::vector<flow::ArcDescriptor>::const_iterator i =
           network.arcs.begin();
       i != network.arcs.end();
       ++i) {
    if (!cut[i->tail] && cut[i->head]) value += i->cap;
  }
  assert(value == solution.value);
}

void VerifyFlow(const char* path, FILE* stream) {
  VerifySolution(path, stream, flow::kFlow);
}

void VerifyPreflow(const char* path, FILE* stream) {
  VerifySolution(path, stream, flow::kPreflow);
}

struct Command {
  const char* name;
  void (*function)(const char* path, FILE* stream);
  const char* description;
};

const Command g_command[] = {
  {"pr", ComputePR,
   "read a network, write a max preflow (push-relabel method)"},
  {"sr", ComputeSR,
   "read a network, write a max preflow (send-relabel method)"},
  {"vc", VerifyCapacityBounds,
   "read a network, verify capacity bounds for nodes and edges"},
  {"vf", VerifyFlow, "read a network and a max flow, verify them"},
  {"vp", VerifyPreflow, "read a network and a max preflow, verify them"},
};

static const size_t kNumCommands = sizeof g_command / sizeof g_command[0];

const Command* Lookup(const char* name) {
  for (size_t i = 0; i < kNumCommands; ++i) {
    if (strcmp(g_command[i].name, name) == 0) return &g_command[i];
  }
  return NULL;
}

void Usage() __attribute__((noreturn));

void Usage() {
  fputs("usage: flow command [file]\n"
        "command is one of\n",
        stderr);
  for (size_t i = 0; i < kNumCommands; ++i) {
    fprintf(stderr, " %s - %s\n", g_command[i].name, g_command[i].description);
  }
  exit(EXIT_FAILURE);
}
}  // namespace

int main(int argc, char** argv) {
  if (!(argc == 2 || argc == 3)) Usage();
  const Command* c = Lookup(argv[1]);
  if (!c) Usage();
  const char* path;
  FILE* stream;
  if (argc == 3) {
    path = argv[2];
    stream = fopen(path, "r");
    if (stream == NULL) {
      perror(path);
      exit(EXIT_FAILURE);
    }
  } else {
    path = "stdin";
    stream = stdin;
  }
  c->function(path, stream);
}
