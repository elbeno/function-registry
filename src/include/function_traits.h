#pragma once

#include <functional>

// A simple function traits template for extracting the first argument type from
// a callable object.

template <typename T>
struct function_traits
  : public function_traits<decltype(&T::operator())>
{};

template <typename R, typename Arg>
struct function_traits<R(Arg)>
{
  typedef Arg argType;
};

template <typename C, typename R, typename Arg>
struct function_traits<R(C::*)(Arg)>
  : public function_traits<R(Arg)>
{};

template <typename C, typename R, typename Arg>
struct function_traits<R(C::*)(Arg) const>
  : public function_traits<R(Arg)>
{};

template <typename F>
struct function_traits<std::function<F>>
  : public function_traits<F>
{};

template <typename T>
struct function_traits<const T>
  : public function_traits<T>
{};

template <typename T>
struct function_traits<volatile T>
  : public function_traits<T>
{};

template <typename T>
struct function_traits<T*>
  : public function_traits<T>
{};

template <typename T>
struct function_traits<T&>
  : public function_traits<T>
{};
