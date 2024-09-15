// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

// Reads points in the unit square from stdin and writes a traveling
// salesman tour to stdout that is (almost) locally optimal (2-opt)
// for the Euclidean metric.  Can be piped to plot.sh.
//
// Croes, "A method for solving traveling-salesman problems"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "dtree/seq.h"
#include "dtree/seq-inl.h"
#include "util/geometry.h"
#include "util/scoped.h"

namespace {

typedef std::vector<util::Point> PointVector;

typedef dtree::WithReverse<dtree::Begin<util::Point> > R;
typedef dtree::EndSeq<R> Node;

inline double Distance(const util::Point* u, const util::Point* v) {
  double dx = u->x - v->x;
  double dy = u->y - v->y;
  return sqrt(dx * dx + dy * dy);
}

void Error(const char* message) __attribute__((noreturn));

void Error(const char* message) {
  fprintf(stderr, "tsp: %s\n", message);
  exit(EXIT_FAILURE);
}
}  // namespace

int main() {
  PointVector points;
  int rv;
  while (true) {
    double x;
    double y;
    rv = scanf("%lg%lg", &x, &y);
    if (rv != 2) break;
    if (x < 0.0 || 1.0 <= x || y < 0.0 || 1.0 <= y) {
      Error("point in the unit square expected");
    }
    points.push_back(util::Point(x, y));
  }
  if (ferror(stdin)) {
    perror("tsp: stdin");
    exit(EXIT_FAILURE);
  } else if (!feof(stdin)) {
    Error("number expected");
  } else if (rv == 1) {
    Error("two numbers expected for point");
  } else if (points.empty()) {
    Error("at least one point expected");
  }
  points.push_back(points.front());
  util::ScopedArray<Node> leftmost(new Node[points.size()]);
  Node* j = leftmost.get();
  for (PointVector::const_iterator i = points.begin();
       i != points.end();
       ++i, ++j) {
    j->x = i->x;
    j->y = i->y;
    if (j != leftmost.get()) LinkLeftOf(j - 1, j);
  }
  Node* rightmost = leftmost.get() + (points.size() - 1);
  bool progress;
  do {
    progress = false;
    Node* u = leftmost.get();
    Node* v = Rightward(u);
    while (v != rightmost) {
      // assuming that sqrt is correctly rounded and points are in the
      // unit square, this tolerance prevents cycling
      double duv =
          Distance(u, v) - 100.0 * std::numeric_limits<double>::epsilon();
      Node* w = Rightward(v);
      while (w != rightmost) {
        Node* x = Rightward(w);
        if (Distance(u, w) + Distance(v, x) < duv + Distance(w, x)) {
          // 2-opt move
          // before: u-v-<<<-w-x
          //  after: u-w->>>-v-x
          //
          // we use one scoped cut and one explicit cut/link pair to
          // demonstrate both features
          dtree::ScopedCutLeftOf<Node> cut(x);
          Node* y = CutRightOf(u);
          R::ReverseSeq(y);
          LinkRightOf(y, u);
          progress = true;
          break;
          // the link v-x is created by the destructor of cut
        }
        w = x;
      }
      u = v;
      v = Rightward(v);
    }
  } while (progress);
  for (Node* j = leftmost.get(); j != NULL; j = Rightward(j)) {
    printf("%.*g\t%.*g\n", std::numeric_limits<double>::digits10, j->x,
           std::numeric_limits<double>::digits10, j->y);
  }
}
