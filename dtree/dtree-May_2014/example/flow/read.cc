// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <limits>
#include <map>
#include <utility>

#include "example/flow/data.h"
#include "example/flow/error.h"

namespace flow {

namespace {

enum NodeDesignator { kS, kT };

struct Position {
  Position();
  // unsigned for defined overflow
  unsigned int line;
  unsigned int column;
};

Position::Position() : line(1), column(1) {
}
}  // namespace

class Reader::ReaderImpl {
 public:
  ReaderImpl(const char* path, FILE* stream);
  void ReadNetwork(Network* network);
  void ReadSolution(const Network& network, Solution* solution);

 private:
  bool ReadRawChar(unsigned char* c);
  bool ReadChar(unsigned char* c);
  void UnreadChar(unsigned char c);
  bool ReadNonSpaceChar(unsigned char* c);
  void ExpectSpace(const char* descr);
  void ExpectEndOfLine();
  void ExpectLineDesignator(const char* descr, unsigned char desig);
  void ExpectProblemDesignator(const char* desig);
  int ReadInt(const char* descr, int min, int max);
  NodeDesignator ReadNodeDesignator();
  void Error(const char* format, ...)
      __attribute__((noreturn, format(printf, 2, 3)));
  void ErrorFirstLine(const char* format, ...)
      __attribute__((format(printf, 2, 3)));
  void NoteSecondLine(unsigned int line, const char* format, ...)
      __attribute__((noreturn, format(printf, 3, 4)));
  const char* path_;
  FILE* stream_;
  Position previous_position_;
  Position position_;
  ErrorWriter ew_;
  DISALLOW_COPY_AND_ASSIGN(ReaderImpl);
};

Reader::ReaderImpl::ReaderImpl(const char* path, FILE* stream)
    : path_(path),
      stream_(stream),
      previous_position_(),
      position_(),
      ew_(path, stderr) {
}

void Reader::ReaderImpl::ReadNetwork(Network* network) {
  network->Reset();
  ExpectLineDesignator("problem line", 'p');
  ExpectProblemDesignator("max");
  network->n =
      ReadInt("number of nodes", 2, std::numeric_limits<int>::max() - 1);
  int m_max = (std::numeric_limits<int>::max() / network->n >= network->n - 1 ?
               network->n * (network->n - 1) :
               std::numeric_limits<int>::max());
  int m = ReadInt("number of arcs", 0, m_max);
  network->arcs.resize(m);
  ExpectEndOfLine();
  unsigned int previous_line = 0;
  for (int i = 0; i < 2; ++i) {
    ExpectLineDesignator("node descriptor", 'n');
    int u = ReadInt("node", 1, network->n);
    NodeDesignator desig = ReadNodeDesignator();
    int* s_or_t;
    const char* descr = "source";
    const char* other_descr = "sink";
    if (desig == kS) {
      s_or_t = &network->s;
    } else if (desig == kT) {
      s_or_t = &network->t;
      std::swap(descr, other_descr);
    } else {
      assert(false);
    }
    bool invalid = false;
    if (*s_or_t != 0) {
      ErrorFirstLine("more than one %s", descr);
      invalid = true;
    }
    *s_or_t = u;
    if (!invalid && network->s == network->t) {
      ErrorFirstLine("%s not distinct from %s", descr, other_descr);
      invalid = true;
    }
    if (invalid) {
      NoteSecondLine(previous_line, "previous node descriptor was here");
    }
    previous_line = position_.line;
    ExpectEndOfLine();
  }
  typedef std::map<std::pair<int, int>, unsigned int> ArcMap;
  ArcMap arc_map_line;
  for (std::vector<ArcDescriptor>::iterator i = network->arcs.begin();
       i != network->arcs.end();
       ++i) {
    ExpectLineDesignator("arc descriptor", 'a');
    ArcDescriptor arc;
    i->tail = ReadInt("tail node", 1, network->n);
    i->head = ReadInt("head node", 1, network->n);
    if (i->tail == i->head) {
      ErrorFirstLine("self-loop");
      exit(EXIT_FAILURE);
    }
    i->cap = ReadInt("arc capacity", 0, std::numeric_limits<int>::max());
    std::pair<ArcMap::const_iterator, bool> result =
        arc_map_line.insert(std::make_pair(std::make_pair(i->tail, i->head),
                                           position_.line));
    if (!result.second) {
      ErrorFirstLine("parallel arc");
      NoteSecondLine(result.first->second, "previous arc descriptor was here");
    }
    ExpectEndOfLine();
  }
  arc_map_line.clear();
  network->Validate();
}

void Reader::ReaderImpl::ReadSolution(const Network& network,
                                      Solution* solution) {
  solution->InitializeFrom(network);
  ExpectLineDesignator("solution line", 's');
  solution->value = ReadInt("objective value", 0,
                            std::numeric_limits<int>::max());
  std::vector<ArcDescriptor>::const_iterator i = network.arcs.begin();
  for (std::vector<int>::iterator j = solution->flows.begin();
       j != solution->flows.end();
       ++i, ++j) {
    ExpectLineDesignator("flow assignment", 'f');
    ReadInt("tail node", i->tail, i->tail);
    ReadInt("head node", i->head, i->head);
    *j = ReadInt("amount of flow", 0, i->cap);
    ExpectEndOfLine();
  }
  solution->Validate(network);
}

bool Reader::ReaderImpl::ReadRawChar(unsigned char* c) {
  int i = getc(stream_);
  if (i == EOF) {
    if (ferror(stream_)) {
      perror(path_);
      exit(EXIT_FAILURE);
    }
    return false;
  }
  *c = i;
  return true;
}

bool Reader::ReaderImpl::ReadChar(unsigned char* c) {
  unsigned char r;
  if (!ReadRawChar(&r)) return false;
  if (r == '\r' && ReadRawChar(&r) && r != '\n') {
    ungetc(r, stream_);
    r = '\n';
  }
  previous_position_ = position_;
  if (r == '\n') {
    position_.column = 1;
    ++position_.line;
  } else if (r == '\t') {
    while (++position_.column % 8 != 1) {}
  } else {
    ++position_.column;
  }
  *c = r;
  return true;
}

void Reader::ReaderImpl::UnreadChar(unsigned char c) {
  position_ = previous_position_;
  ungetc(c, stream_);
}

bool Reader::ReaderImpl::ReadNonSpaceChar(unsigned char* c) {
  unsigned char r;
  if (!ReadChar(&r)) return false;
  if (r == ' ' || r == '\n' || r == '\t') {
    UnreadChar(r);
    return false;
  }
  *c = r;
  return true;
}

void Reader::ReaderImpl::ExpectSpace(const char* descr) {
  bool no_space = true;
  unsigned char c;
  while (ReadChar(&c)) {
    if (c == ' ' || c == '\t') {
      no_space = false;
      continue;
    }
    UnreadChar(c);
    if (c == '\n') Error("%s expected before end of line", descr);
    if (no_space) Error("space expected before %s", descr);
    return;
  }
  Error("%s expected before end of file", descr);
}

void Reader::ReaderImpl::ExpectEndOfLine() {
  unsigned char c;
  while (ReadChar(&c)) {
    if (c == '\n') return;
    if (c != ' ' && c != '\t') {
      UnreadChar(c);
      Error("end of line expected");
    }
  }
  Error("end of line expected before end of file");
}

void Reader::ReaderImpl::ExpectLineDesignator(const char* descr,
                                              unsigned char desig) {
  unsigned char c;
  while (ReadChar(&c)) {
    if (c == ' ' || c == '\n' || c == '\t') continue;
    if (c == desig) return;
    if (c != 'c') {
      UnreadChar(c);
      Error("%s '%c' or comment line 'c' expected", descr, desig);
    }
    while (ReadChar(&c) && c != '\n') {}
  }
  Error("%s '%c' expected before end of file", descr, desig);
}

void Reader::ReaderImpl::ExpectProblemDesignator(const char* desig) {
  const char* descr = "problem designator";
  ExpectSpace(descr);
  bool invalid = false;
  Position start = position_;
  unsigned char c;
  for (const char* p = desig; *p != '\0'; ++p) {
    if (ReadNonSpaceChar(&c) && c == *p) continue;
    invalid = true;
    break;
  }
  if (invalid || ReadNonSpaceChar(&c)) {
    position_ = start;
    Error("%s \"%s\" expected", descr, desig);
  }
}

int Reader::ReaderImpl::ReadInt(const char* descr, int min, int max) {
  ExpectSpace(descr);
  bool invalid = false;
  Position start = position_;
  div_t q = div(max, 10);
  int i = 0;
  unsigned char c;
  while (ReadNonSpaceChar(&c)) {
    if (c < '0' || '9' < c) {
      invalid = true;
      break;
    }
    int d = c - '0';
    if (i > q.quot || (i == q.quot && d > q.rem)) {
      invalid = true;
      break;
    }
    i = 10 * i + d;
  }
  if (invalid || i < min) {
    position_ = start;
    Error("%s: integer between %d and %d expected", descr, min, max);
  }
  return i;
}

NodeDesignator Reader::ReaderImpl::ReadNodeDesignator() {
  const char* descr = "node designator";
  ExpectSpace(descr);
  unsigned char c;
  ReadChar(&c);
  if (c == 's') return kS;
  if (c == 't') return kT;
  UnreadChar(c);
  Error("%s: 's' for source or 't' for sink expected", descr);
}

void Reader::ReaderImpl::Error(const char* format, ...) {
  ew_.Error(position_.line, position_.column, kError);
  va_list ap;
  va_start(ap, format);
  ew_.FormatArgs(format, ap);
  va_end(ap);
  ew_.Format("\n");
  exit(EXIT_FAILURE);
}

void Reader::ReaderImpl::ErrorFirstLine(const char* format, ...) {
  ew_.Error(position_.line, 0, kError);
  va_list ap;
  va_start(ap, format);
  ew_.FormatArgs(format, ap);
  va_end(ap);
  ew_.Format("\n");
}

void Reader::ReaderImpl::NoteSecondLine(unsigned int line,
                                        const char* format,
                                        ...) {
  ew_.Error(line, 0, kNote);
  va_list ap;
  va_start(ap, format);
  ew_.FormatArgs(format, ap);
  va_end(ap);
  ew_.Format("\n");
  exit(EXIT_FAILURE);
}

Reader::Reader(const char* path, FILE* stream)
    : impl_(new ReaderImpl(path, stream)) {
}

Reader::~Reader() {
  delete impl_;
}

void Reader::ReadNetworkOrDie(Network* network) {
  impl_->ReadNetwork(network);
}

void Reader::ReadSolutionOrDie(const Network& network, Solution* solution) {
  impl_->ReadSolution(network, solution);
}
}  // namespace flow
