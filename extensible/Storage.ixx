module;
#include <concepts>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
export module Mitama.Data.Extensible.Named:Storage;

namespace mitama {
  // primary
  // owned storage
  template <class T>
  class named_storage {
    T value;
  public:
    named_storage() = delete;
    constexpr named_storage(named_storage const&) = default;
    constexpr named_storage(named_storage&&) = default;
    constexpr named_storage& operator=(named_storage const&) = default;
    constexpr named_storage& operator=(named_storage&&) = default;

    // basic constructor
    template <class ...Args>
    requires std::constructible_from<T, Args...>
      constexpr explicit named_storage(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : value(std::forward<Args>(args)...)
    {}

    template <class U, class ...Args>
    requires std::constructible_from<T, Args...>
      constexpr explicit named_storage(std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...>)
      : value( il, std::forward<Args>(args)... )
    {}

    constexpr decltype(auto) deref()& { return value; }
    constexpr decltype(auto) deref() const& { return value; }

    constexpr decltype(auto) indirect()& { return std::addressof(value); }
    constexpr decltype(auto) indirect() const& { return std::addressof(value); }
  };

  // specialization
  // borrowed storage
  template <class T>
  class named_storage<T&> {
    std::reference_wrapper<T> ref;
  public:
    named_storage() = delete;
    constexpr named_storage(named_storage const&) = default;
    constexpr named_storage(named_storage&&) = default;
    constexpr named_storage& operator=(named_storage const&) = default;
    constexpr named_storage& operator=(named_storage&&) = default;

    // basic constructor
    template <class U>
    requires std::constructible_from<std::reference_wrapper<T>, U>
      constexpr named_storage(U&& from)
      noexcept(std::is_nothrow_constructible_v<std::reference_wrapper<T>, U>)
      : ref{ from }
    {}

    constexpr decltype(auto) deref()& { return ref.get(); }
    constexpr decltype(auto) deref() const& { return ref.get(); }

    constexpr decltype(auto) indirect()& { return std::addressof(ref.get()); }
    constexpr decltype(auto) indirect() const& { return std::addressof(ref.get()); }
  };
}
