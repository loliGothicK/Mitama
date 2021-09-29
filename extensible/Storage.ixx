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

    // delegating constructor for in place construction
    template <class ...Args>
    requires std::constructible_from<T, Args...>
      constexpr explicit named_storage(std::tuple<Args...> into)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage{ into, std::index_sequence_for<Args...>{} }
    {}

  private:
    // called between delegating constructor and basic constructor
    template <class ...Args, std::size_t ...Indices>
    requires std::constructible_from<T, Args...>
      constexpr explicit named_storage(std::tuple<Args...> into, std::index_sequence<Indices...>)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage{ std::get<Indices>(into)... }
    {}

  public:
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
