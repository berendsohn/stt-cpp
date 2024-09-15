// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef DTREE_NAIVE_TREE_H_
#define DTREE_NAIVE_TREE_H_

#include "dtree/naive/common.h"

namespace naive {

template<typename Base>
class EndTree : public Base {
 public:
  EndTree() : parent_(NULL) {}
  EndTree* parent() const { return parent_; }
  void set_parent(EndTree* parent) { parent_ = parent; }
  template<typename Forwarder>
  void CopyFrom(const EndTree& node, Forwarder forward);
 private:
  EndTree* parent_;
  DISALLOW_COPY_AND_ASSIGN(EndTree);
};

template<typename Type, typename Base>
class WithStaticValue : public Base {
 public:
  typedef dtree::Nop<Type> Group_;
  Type value() const { return value_; }
  void set_value(Type value) { value_ = value; }
  //
  template<typename Node>
  static Type Value(Node* node);
  template<typename Node>
  static void SetValue(Node* node, Type value);
 protected:
  WithStaticValue() : value_() {}
 private:
  Type value_;
};

template<typename Group, typename Base>
class WithAncValue : public Base {
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
  //
  template<typename Node>
  static typename Group::Type Value(Node* node);
  template<typename Node>
  static void SetValue(Node* node, typename Group::Type value);
  template<typename Node>
  static void AddToAnc(Node* node, typename Group::Type delta);
  template<typename Node>
  static void SubtractFromAnc(Node* node, typename Group::Type delta);
  template<typename Node>
  static void AddToProperAnc(Node* node, typename Group::Type delta);
  template<typename Node>
  static void SubtractFromProperAnc(Node* node, typename Group::Type delta);

 protected:
  WithAncValue() : value_() {}

 private:
  typename Group::Type value_;
};

template<typename Semigroup, typename Base>
class WithAncAggr : public Base {
 public:
  typedef Semigroup Semigroup_;
  typename Semigroup::Type singleton_aggr() const {
    return Semigroup::AggrFromValue(this->value());
  }
  //
  template<typename Node>
  static typename Semigroup::Type AggrAnc(Node* node);
  template<typename Node, typename Predicate>
  static Node* FindDirmostAnc(int dir, Node* node, const Predicate& predicate);
  template<typename Node, typename Predicate>
  static Node* FindLeafmostAnc(Node* node, const Predicate& predicate);
  template<typename Node, typename Predicate>
  static Node* FindRootmostAnc(Node* node, const Predicate& predicate);
  template<typename Node>
  static typename Semigroup::Type AggrProperAnc(Node* node);
  template<typename Node, typename Predicate>
  static Node* FindDirmostProperAnc(int dir,
                                    Node* node,
                                    const Predicate& predicate);
  template<typename Node, typename Predicate>
  static Node* FindLeafmostProperAnc(Node* node, const Predicate& predicate);
  template<typename Node, typename Predicate>
  static Node* FindRootmostProperAnc(Node* node, const Predicate& predicate);

 protected:
  WithAncAggr() {}
};

template<typename Base>
class WithEvertBy : public Base {
 private:
  typedef typename Base::Group_ Group;
 public:
  template<typename Node>
  static void Evert(Node* node);
 protected:
  WithEvertBy() {}
};

template<typename Base>
class WithEvert : public Base {
 public:
  template<typename Node>
  static void Evert(Node* node);
 protected:
  WithEvert() {}
};
}  // namespace naive
#endif  // DTREE_NAIVE_TREE_H_
