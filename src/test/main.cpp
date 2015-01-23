#include "type_key.h"

// -----------------------------------------------------------------------------
// Specialize type_key to not use RTTI: just provide a unique id per type.

template <>
struct type_key<void>
{
  using type = int;
  static type key() { return 0; }
};

// -----------------------------------------------------------------------------
#include "registry.h"

#include <iostream>
using namespace std;

// -----------------------------------------------------------------------------
// A couple of unrelated message types

struct MessageA
{
};

struct MessageB
{
};

// -----------------------------------------------------------------------------
// Specializations of type_key

template <>
struct type_key<MessageA>
{
  using type = int;
  static type key() { return 1; }
};

template <>
struct type_key<MessageB>
{
  using type = int;
  static type key() { return 2; }
};

// -----------------------------------------------------------------------------
// Some functions/callable objects that handle those types

void f(const MessageA&)
{
  cout << "f(const MessageA&) called" << endl;
}

void g(const MessageB&)
{
  cout << "g(const MessageB&) called" << endl;
}

void byvalue_f(MessageA)
{
  cout << "byvalue_f(const MessageA&) called" << endl;
}

void byref_f(MessageA&)
{
}

class Foo
{
public:
  void f(const MessageA&)
  {
    cout << "Foo::f(const MessageA&) called" << endl;
  }
};

// -----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  Registry r;

  // Ordinary functions
  r.Register(f);
  r.Register(byvalue_f);
  //r.Register(byref_f); // fails to compile (as it should)

  // Lambdas
  r.Register([] (const MessageA&)
             { cout << "lambda (const MessageA&) called" << endl; });
  Registry::Token t = r.Register([] (const MessageA&) mutable
                                 { cout << "mutable lambda (const MessageA&) called" << endl; });

  // std::functions (made from member functions)
  Foo foo;
  std::function<void(const MessageA&)> foo_f = [&foo] (const MessageA& a) { foo.f(a); };
  r.Register(foo_f);

  r.Register(g);

  cout << "Dispatch MessageA" << endl;

  MessageA a;
  r.Dispatch(a);

  cout << endl << "Dispatch MessageB" << endl;

  MessageB b;
  r.Dispatch(b);

  cout << endl << "Unregister mutable lambda" << endl;
  r.Unregister(t);
  cout << "Dispatch MessageA" << endl;
  r.Dispatch(a);

  return 0;
}
