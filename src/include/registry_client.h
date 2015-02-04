#pragma once

#include "registry.h"

// Function registry client: a helper class to automate usage of a function
// registry. Handles storage of tokens for unregistering by type and cleanup on
// deletion.

bool operator<(const Registry::Token& a, const Registry::Token& b)
{
  return a.m_id < b.m_id;
}

class RegistryClient
{
  using Token = Registry::Token;

public:
  explicit RegistryClient(Registry* r)
    : m_registry(r)
  {}

  virtual ~RegistryClient()
  {
    UnregisterAll();
  }

  // Register a function, and store the token.
  template <typename F>
  void Register(F&& f)
  {
    m_tokens.push_back(
        m_registry->Register(std::forward<F>(f)));
  }

  // Unregister a single token.
  void Unregister(const Token& t)
  {
    auto i = std::lower_bound(m_tokens.cbegin(), m_tokens.cend(), t.m_id,
                              [] (const Token& t, auto id)
                              { return t.m_id < id; });
    if (i != m_tokens.cend() && i->m_id == t.m_id)
    {
      m_registry->Unregister(t);
      m_tokens.erase(i);
    }
  }

  // Unregister a function (the first one found) associated with a type.
  template <typename T>
  void Unregister()
  {
    auto i = std::find_if(m_tokens.cbegin(), m_tokens.cend(),
                          [] (const Token& t)
                          { return t.m_k == type_key<T>::key(); });
    if (i != m_tokens.end())
    {
      m_registry->Unregister(*i);
      m_tokens.erase(i);
    }
  }

  void UnregisterAll()
  {
    std::for_each(m_tokens.cbegin(), m_tokens.cend(),
                  [this] (const Token& t) { m_registry->Unregister(t); });
    m_tokens.clear();
  }

private:
  Registry* m_registry;
  std::vector<Token> m_tokens;
};
