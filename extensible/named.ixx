module;
#include <compare>
#include <concepts>
#include <functional>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
export module Mitama.Data.Extensible.Named;
import Mitama.Base.Concepts.DataKind;
import Mitama.Functional.Extensible;
import Mitama.Utility.Extensible;
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
    using value_type = T;

    constexpr named() = delete;
    constexpr named(named const&) = default;
    constexpr named(named&&) = default;
    constexpr named& operator=(named const&) = default;
    constexpr named& operator=(named&&) = default;

    template <class ...Args> requires std::constructible_from<T, Args...>
    constexpr explicit named(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage<T>{ std::forward<Args>(args)... }
    {}

    template <class U, class ...Args> requires
      std::constructible_from<T, std::initializer_list<U>, Args...>
    constexpr explicit named(std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...>)
      : named_storage<T>{ il, std::forward<Args>(args)... }
    {}

    template <auto Id, class U>
      requires (std::constructible_from<T, U>
            && (not std::is_same_v<decltype(Id), decltype(Tag)>)
            && (decltype(Tag)::value == decltype(Id)::value))
    constexpr explicit(!std::is_convertible_v<U, T>)
    named(named<Id, U> from)
      noexcept(std::is_nothrow_constructible_v<T, U>)
      : named_storage<T>{ from.value() }
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
    template <auto S> requires (static_string<S>::value == str)
    constexpr decltype(auto) operator[](static_string<S>) const noexcept {
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

    template <class T>
    constexpr auto operator=(T&& val) const {
      return named<Tag, T>(std::forward<T>(val));
    }

    template <class T>
    constexpr auto operator%(T&& val) const {
      return named<Tag, T>(std::forward<T>(val));
    }
  };

  // Overloading to make it easier to build `named`.
  // [ Example:
  //    ```cpp
  //    named<"id"_, int> id = as<"id"_>(42); 
  //    ```
  //    -- end example ]
  template <static_string S, class T>
  inline constexpr auto as(T&& x) { return named<S, T>{ std::forward<T>(x) }; }

  template <auto S1, auto S2, class T1, class T2>
  constexpr auto operator+=(named<S1, T1> const& fst, named<S2, T2> const& snd) {
    return std::tuple{ fst, snd };
  }

  template <auto S, class T, class ...Tail>
  constexpr auto operator+=(named<S, T> const& fst, std::tuple<Tail...> tail) {
    return std::apply([&](auto&&... tail) {
      return std::tuple{fst, std::forward<decltype(tail)>(tail)...};
    }, tail);
  }
}

export namespace mitama:: inline literals:: inline static_string_literals {
  // named variable literal
  template<fixed_string S>
  inline constexpr auto operator""_v() {
    return named<static_string<S>{}>();
  }
}
