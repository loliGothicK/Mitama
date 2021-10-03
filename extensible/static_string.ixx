module;
#include <cstddef>
#include <string_view>
#include <type_traits>
export module Mitama.Data.Extensible.StaticString;
import Mitama.Utility.Extensible;

export namespace mitama {
  template<std::size_t N, class CharT>
  struct fixed_string {
    static constexpr std::size_t size = N;
    using char_type = CharT;

    constexpr fixed_string(CharT const (&s)[N])
      : fixed_string(s, std::make_index_sequence<N>{}) {}
    template<std::size_t ...Indices>
    constexpr fixed_string(CharT const (&s)[N], std::index_sequence<Indices...>)
      : s{ s[Indices]... } {}

    CharT const s[N];
  };
}

export namespace mitama {
  // non-type template enabled static string class
  // 
  // S: fixed_string<N, CharT>
  template<fixed_string S>
  struct static_string {
    using char_type = typename decltype(S)::char_type;
    static constexpr std::basic_string_view<char_type> const value = { S.s, decltype(S)::size };

    template <auto T>
    requires std::same_as<char_type, typename static_string<T>::char_type>
      constexpr std::strong_ordering operator<=>(static_string<T>) const noexcept {
      return static_string::value <=> static_string<T>::value;
    }

    constexpr operator std::basic_string_view<char_type>() const {
      return value;
    }
  };
}

export namespace mitama:: inline literals:: inline static_string_literals{
  // static string literal
  template<fixed_string S>
  inline constexpr static_string<S> operator""_() {
    return {};
  }
}
