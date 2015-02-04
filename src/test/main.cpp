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
#include "registry_client.h"

#include <cassert>
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
// Test counts

namespace
{
  int a_calls = 0;
  int b_calls = 0;
};

// -----------------------------------------------------------------------------
// Some functions/callable objects that handle those types

void f(const MessageA&)
{
  ++a_calls;
}

void g(const MessageB&)
{
  ++b_calls;
}

void byvalue_f(MessageA)
{
  ++a_calls;
}

void byref_f(MessageA&)
{
  ++a_calls;
}

class Foo
{
public:
  void f(const MessageA&)
  {
    ++a_calls;
  }
};

// -----------------------------------------------------------------------------
void test()
{
  Registry r;

  // Ordinary functions
  r.Register(f);
  r.Register(byvalue_f);
  //r.Register(byref_f); // fails to compile (as it should)

  // Lambdas
  r.Register([] (const MessageA&) { ++a_calls; });
  Registry::Token t = r.Register([] (const MessageA&) mutable { ++a_calls; });

  // std::functions
  Foo foo;
  std::function<void(const MessageA&)> foo_f =
    [&foo] (const MessageA& a) { foo.f(a); };
  r.Register(foo_f);

  // g handles MessageB
  r.Register(g);

  MessageA a;
  r.Dispatch(a);
  assert(a_calls == 5);
  assert(b_calls == 0);

  MessageB b;
  r.Dispatch(b);
  assert(a_calls == 5);
  assert(b_calls == 1);

  a_calls = b_calls = 0;

  r.Unregister(t);
  r.Dispatch(a);
  assert(a_calls == 4);
  assert(b_calls == 0);

  a_calls = b_calls = 0;
}

class TestClient : public RegistryClient
{
public:
  TestClient(Registry* r) : RegistryClient(r) {}
};

void testClient()
{
  Registry r;

  {
    TestClient c(&r);
    c.Register(f);

    r.Dispatch(MessageA());
    assert(a_calls == 1);
    assert(b_calls == 0);

    c.Unregister<MessageA>();
    r.Dispatch(MessageA());
    assert(a_calls == 1);
    assert(b_calls == 0);
  }

  a_calls = b_calls = 0;

  {
    TestClient c(&r);
    c.Register(f);
    c.Register(g);

    r.Dispatch(MessageA());
    assert(a_calls == 1);
    assert(b_calls == 0);
    r.Dispatch(MessageB());
    assert(a_calls == 1);
    assert(b_calls == 1);
  }

  r.Dispatch(MessageA());
  r.Dispatch(MessageB());
  assert(a_calls == 1);
  assert(b_calls == 1);

  a_calls = b_calls = 0;
}

int main(int argc, char* argv[])
{
  test();
  testClient();

  cout << "All tests passed." << endl;
  return 0;
}
