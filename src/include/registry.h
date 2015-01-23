#pragma once

#include "function_traits.h"
#include "type_key.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

// Function registry: register functions of type void(const T&) then dispatch
// objects of type T to them. (T may vary.) Functions types are matched up with
// object types so that the right functions get called when dispatching an
// object.

class Registry
{
  template <typename F>
  using arg_t = std::decay_t<typename function_traits<F>::argType>;

  using typekey_t = typename type_key<void>::type;
  using id_t = int;

  class HandlerWrapper
  {
  public:
    template <typename F>
    HandlerWrapper(F&& f, id_t id)
      : m_internal(std::make_unique<Internal<F>>(std::forward<F>(f)))
      , m_id(id)
    {}

    HandlerWrapper(HandlerWrapper&& other)
      : m_internal(std::move(other.m_internal))
      , m_id(std::move(other.m_id))
    {}

    HandlerWrapper& operator=(HandlerWrapper&& other)
    {
      m_internal = std::move(other.m_internal);
      m_id = std::move(other.m_id);
      return *this;
    }

    template <typename T>
    void operator()(const T& t) const
    {
      m_internal->Call(static_cast<const void*>(&t));
    }

    id_t id() const { return m_id; }

  private:
    class InternalBase
    {
    public:
      virtual ~InternalBase() {}
      virtual void Call(const void* pt) const = 0;
    };

    template <typename F>
    class Internal : public InternalBase
    {
    public:
      Internal(F&& f) : m_f(std::forward<F>(f))
      {
      }

      virtual void Call(const void* pt) const
      {
        m_f(*static_cast<const arg_t<F>*>(pt));
      }

      F m_f;
    };

    std::unique_ptr<InternalBase> m_internal;
    id_t m_id;
  };

public:
  struct Token
  {
    // Tokens have a default constructor so that clients may store them in
    // containers.
    Token()
      : m_k(type_key<void>::key())
      , m_id(id_t())
    {}
    Token(typekey_t k, id_t id)
      : m_k(k)
      , m_id(id)
    {}

    typekey_t m_k;
    id_t m_id;
  };

  Registry() : m_handlerId(id_t())
  {}

  // Register a function. Returns a token that may be used to unregister the
  // function.
  template <typename F>
  Token Register(F&& f)
  {
    auto k = type_key<arg_t<F>>::key();
    m_handlers[k].push_back(HandlerWrapper(std::forward<F>(f), m_handlerId));
    return {k, m_handlerId++};
  }

  // Unregister a function (if any) associated with a token.
  void Unregister(const Token& t)
  {
    auto& v = m_handlers[t.m_k];
    auto i = std::lower_bound(v.cbegin(), v.cend(), t.m_id,
                              [] (const HandlerWrapper& h, int id)
                              { return h.id() < id; });
    if (i != v.cend() && i->id() == t.m_id)
      v.erase(i);
  }

  // Dispatch a message. Any functions that can take that type as an argument
  // will be called.
  template <typename T>
  void Dispatch(const T& t)
  {
    auto k = type_key<std::decay_t<T>>::key();
    auto& v = m_handlers[k];
    std::for_each(v.cbegin(), v.cend(),
                  [&t] (const HandlerWrapper& h) { h(t); });
  }

private:
  std::unordered_map<typekey_t, std::vector<HandlerWrapper>> m_handlers;
  id_t m_handlerId;
};
