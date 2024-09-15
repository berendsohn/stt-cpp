// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_NAIVE_SEQ_H_
#define DTREE_NAIVE_SEQ_H_

#include "dtree/naive/common.h"

namespace naive {

template<typename Base>
class EndSeq : public Base {
 public:
  EndSeq() : solid_() {}
  EndSeq* solid(int i) const { return solid_[i]; }
  void set_solid(int i, EndSeq* solid_i) { solid_[i] = solid_i; }
  template<typename Forwarder>
  void CopyFrom(const EndSeq& node, Forwarder forward);
 private:
  EndSeq* solid_[2];
  DISALLOW_COPY_AND_ASSIGN(EndSeq);
};

template<typename Group, typename Base>
class WithValue : public Base {
 public:
  typedef Group Group_;
  typename Group::Type value() const { return value_; }
  void set_value(typename Group::Type value) { value_ = value; }
  void add_to_value(typename Group::Type delta) {
    set_value(Group::Plus(value(), delta));
  }
  void subtract_from_value(typename Group::Type delta) {
    set_value(Group::Minus(value(), delta));
  }
  template<typename Node>
  static typename Group::Type Value(Node* node);
  template<typename Node>
  static void SetValue(Node* node, typename Group::Type value);
  template<typename Node>
  static void AddToSeq(Node* node, typename Group::Type delta);
  template<typename Node>
  static void SubtractFromSeq(Node* node, typename Group::Type delta);

 protected:
  WithValue() : value_() {}

 private:
  typename Group::Type value_;
};

template<typename Semigroup, typename Base>
class WithAggr : public Base {
 public:
  typedef Semigroup Semigroup_;
  typename Semigroup::Type singleton_aggr() const {
    return Semigroup::AggrFromValue(this->value());
  }
  template<typename Node>
  static typename Semigroup::Type AggrSeq(Node* node);
  template<typename Node, typename Predicate>
  static Node* FindDirmostSeq(int dir, Node* node, const Predicate& predicate);
  template<typename Node, typename Predicate>
  static Node* FindLeftmostSeq(Node* node, const Predicate& predicate);
  template<typename Node, typename Predicate>
  static Node* FindRightmostSeq(Node* node, const Predicate& predicate);
 protected:
  WithAggr() {}
};

template<typename Base>
class WithReverseBy : public Base {
 private:
  typedef typename Base::Group_ Group;
 public:
  template<typename Node>
  static void ReverseSeq(Node* node);
 protected:
  WithReverseBy() {}
};

template<typename Base>
class WithReverse : public Base {
 public:
  template<typename Node>
  static void ReverseSeq(Node* node);
 protected:
  WithReverse() {}
};
}  // namespace naive
#endif  // DTREE_NAIVE_SEQ_H_
