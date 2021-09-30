module;
#include <compare>
#include <concepts>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
export module Mitama.Data.Extensible.Named;
import Mitama.Functional.Extensible;
import Mitama.Data.Extensible.Into;
import Mitama.Data.Extensible.StaticString;
import Mitama.Data.Extensible.TypeList;
import :Storage;

export namespace mitama {
  // Opaque-type that strict-typed via a phantom-parameter `Tag`.
  template <static_string Tag, class T = std::void_t<>>
  class named : named_storage<T> {
    using storage = named_storage<T>;
  public:
    static constexpr std::string_view str = decltype(Tag)::value;
    static constexpr static_string tag = Tag;

    constexpr named() = delete;
    constexpr named(named const&) = default;
    constexpr named(named&&) = default;
    constexpr named& operator=(named const&) = default;
    constexpr named& operator=(named&&) = default;

    // basic constructor (for direct init)
    template <class U> requires std::constructible_from<T, U>
    constexpr explicit(!std::is_convertible_v<U, T>)
    named(U&& from)
      noexcept(std::is_nothrow_constructible_v<T, U>)
      : named_storage<T>{ std::forward<U>(from) }
    {}

    // in place constructor
    template <class ...Args> requires std::constructible_from<T, Args...>
    constexpr named(into_<Tag, Args...> into)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage<T>{ into.args }
    {}

    // accessors
    constexpr decltype(auto) value()& { return storage::deref(); }
    constexpr decltype(auto) value() const& { return storage::deref(); }

    // dereference
    constexpr auto operator*() -> T {
      return storage::deref();
    }

    // indirections
    auto operator->() & -> std::decay_t<T>* {
      return storage::indirect();
    }
    auto operator->() const& -> std::decay_t<T> const* {
      return storage::indirect();
    }

    // clone
    constexpr auto clone() const { return *this; }

  protected:
    // for records
    constexpr auto operator[](decltype(Tag)) const noexcept -> T {
      return storage::deref();
    }
  };

  template <static_string Tag>
  class named<Tag, void> {
  public:
    static constexpr std::string_view str = decltype(Tag)::value;
    static constexpr static_string tag = Tag;

    template <static_string S>
    constexpr std::strong_ordering operator<=>(named<S>) const noexcept {
      return named::str <=> named<S>::str;
    }
  };

  // Overloading to make it easier to build `named`.
  // [ Example:
  //    ```cpp
  //    named<"id"_, int> id = "id"_ <= 42; 
  //    ```
  //    -- end example ]
  template <auto S, class T>
  constexpr auto operator<<(static_string<S>, T&& x) noexcept {
    return named<default_v<static_string<S>>, T>{ std::forward<T>(x) };
  }
}
