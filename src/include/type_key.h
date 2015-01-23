#pragma once

#include <typeindex>

// A struct that can be specialized to provide a hashable unique identifier for
// a type. By default, use type_index.

template <typename T>
struct type_key
{
  using type = std::type_index;
  static type key() { return type(typeid(T)); }
};
