module;

#include <variant>
#include <concepts>
#include <type_traits>
#include <iostream>

export module Mitama.Data.Maybe.def;
export import :just;
export import :nothing;
import Mitama.Data.Panic;
using namespace std;

namespace mitama::customisation_point::for_maybe {
  template <class T>
  inline constexpr auto _transpose(T&& res) { return transpose(std::forward<T>(res)); }
}

namespace mitama {
  // class maybe
  export template <destructible T>
  class maybe {
    variant<nothing_t, just_t<T>> storage;
  public:
    // associated types
    using value_type = remove_cvref_t<T>;

    // constructor
    template <class U>
      requires constructible_from<T, U>
    constexpr maybe(just_t<U> const& u) : storage{ in_place_type<just_t<T>>, u.value }
    {}

    // constructor
    constexpr maybe(nothing_t) : storage{ nothing } {}

    // default constructor
    constexpr maybe() : maybe(nothing) {}

    // special member functions
    maybe(maybe const&) = default;
    maybe& operator=(maybe const&) = default;
    maybe(maybe&&) = default;
    maybe& operator=(maybe&&) = default;

    // operator bool (able to use contextually convertible to bool)
    constexpr explicit operator bool() const noexcept {
      return !holds_alternative<nothing_t>(storage);
    }

    constexpr value_type& unwrap() & {
      if (bool(*this)) {
        return get<just_t<T>>(storage).get();
      }
      else {
        throw runtime_panic{ "unwrap with nothing value." };
      }
    }
    constexpr const value_type& unwrap() const& {
      if (bool(*this)) {
        return get<just_t<T>>(storage).get();
      }
      else {
        throw runtime_panic{ "unwrap with nothing value." };
      }
    }
    constexpr value_type&& unwrap() && {
      if (bool(*this)) {
        return get<just_t<T>>(storage).get();
      }
      else {
        throw runtime_panic{ "unwrap with nothing value." };
      }
    }
    constexpr const value_type&& unwrap() const&& {
      if (bool(*this)) {
        return get<just_t<T>>(storage).get();
      }
      else {
        throw runtime_panic{ "unwrap with nothing value." };
      }
    }

    constexpr auto transpose() const {
      return customisation_point::for_maybe::_transpose(*this);
    }
  };

  // deduction guides
  export template <class T> maybe(just_t<T>&)      ->  maybe<T>;
  export template <class T> maybe(just_t<T>const&) ->  maybe<const T>;
  export template <class T> maybe(just_t<T>&&)     ->  maybe<remove_reference_t<T>>;

  namespace mitamagic {
    template <class>
    struct is_maybe : std::false_type {};

    template <class T>
    struct is_maybe<maybe<T>> : std::true_type {};
  }

  export template <class T>
  concept is_maybe = mitamagic::is_maybe<remove_cvref_t<T>>::value;

} //! namespace mitama
