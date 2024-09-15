// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

// This standard-conforming C++ program captures the inheritance
// pattern used by the dtree library.  It is accepted by recent
// versions of GCC, Clang, and Intel C++ Compiler, but not Microsoft
// Visual C++ 2010.

class Begin {
};

template<typename Group, typename Base>
class WithValue : public Base {
 public:
  typedef Group Group_;
};

template<typename Base>
class WithAggr : public Base {
 private:
  typedef typename Base::Group_ Group;
};

int main() {
  WithAggr<WithValue<double, WithAggr<WithValue<int, Begin> > > > node;
}
