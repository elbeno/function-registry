## The problem of notification dispatch

You have several message types, let's call two of them `A` and `B`.

```c++
struct A { ... };
struct B { ... };
```

In general the types of messages are unrelated. You want some way to register
handlers for these notifications, where the handlers take an `A` or a `B`
respectively:

```c++
void handleA(const A& a) { ... }
void handleB(const B& b) { ... }
```

Of course, these handlers can be regular functions, lambdas, `std::function`s,
classes with `operator()`... in general just something that is callable.

So on the handler side, you want to have some kind of registry `r`, and write
something like:

```c++
r.Register(handleA);
r.Register(handleB);
```

Conversely on the notification dispatch side, you want that to be simple as
well. You just want to write:

```c++
r.Dispatch(A(...));
r.Dispatch(B(...));
```

And have `handleA` called in the first instance and `handleB` called in the
second.

That's what this little library lets you do. Take a look at
[src/test/main.cpp](https://github.com/elbeno/function-registry/blob/master/src/test/main.cpp)
for more examples.

## Limitations

* Thread safety is not addressed (although it would be fairly easy to add).
* RTTI (in the form of `typeid`) is used by default. However, if you have an
  aversion to RTTI, and a fixed set of message types, you can specialize
  `type_key` for each type and provide a compile-time constant that can be used
  as a map key. (This is illustrated in
  [src/test/main.cpp](https://github.com/elbeno/function-registry/blob/master/src/test/main.cpp)).
* Message types and functions argument types must match up: no polymorphism of
  arguments is supported. (If `Foo` inherits from `Bar` and your handler takes a
  `const Bar&`, dispatching a `Foo` won't call it.) This works best for POD
  types.
* The code is written for a standards-compliant C++14 compiler, although with
  only a few changes, C++98 would work (a little less efficiently and
  elegantly).
