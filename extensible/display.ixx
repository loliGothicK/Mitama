module;
#include <iostream>
#include <format>
#include <tuple>
#include <type_traits>
#include <ranges>
export module Mitama.Base.Data.Display;
import Mitama.Base.Data.Text;

export namespace mitama {
  template <class T>
  concept display = requires { std::formatter<T>; };

  template <class T>
  struct is_display: std::bool_constant<display<T>> {};
}

export namespace mitama {
  template <class T>
  concept tuple_like = requires (T t) {
    std::tuple_size<std::remove_cvref_t<T>>::value;
    std::get<0>(t);
  };

  template <class Dict>
  concept dictionary = requires {
    typename std::remove_cvref_t<Dict>::key_type;
    typename std::remove_cvref_t<Dict>::mapped_type;
    requires requires (Dict dict, typename std::remove_cvref_t<Dict>::key_type key) {
      { dict[key] } -> std::same_as<typename std::remove_cvref_t<Dict>::mapped_type>;
    };
  };
}

namespace mitama {
  template <class, template <class> class, class>
  struct requires_all_impl : std::false_type {};

  template <class T, template <class> class Pred, std::size_t... I>
  struct requires_all_impl<T, Pred, std::index_sequence<I...>>
    : std::conjunction<Pred<std::remove_cvref_t<decltype(std::get<I>(std::declval<T>()))>>...>
  {};

  template <class Tuple, template <class> class Pred>
  concept requires_all
    = requires_all_impl<Tuple, Pred, std::make_index_sequence<std::tuple_size_v<Tuple>>>::value;

  template <class T>
  struct not_array_impl : std::true_type {};

  template <class T, std::size_t N>
  struct not_array_impl<std::array<T, N>> : std::false_type {};

  template <class T>
  concept not_array = not_array_impl<std::remove_cvref_t<T>>::value;

  template <class T>
  concept not_string = not requires (T x) { { x } -> std::convertible_to<std::string_view>; }
                    && not requires (T x) { { x } -> std::convertible_to<std::u8string_view>; }
                    && not requires (T x) { { x } -> std::convertible_to<std::u16string_view>; }
                    && not requires (T x) { { x } -> std::convertible_to<std::u32string_view>; }
                    && not requires (T x) { { x } -> std::convertible_to<std::wstring_view>; };
}

export namespace std {
  template <mitama::tuple_like T>
    requires mitama::requires_all<T, mitama::is_display>
          && mitama::not_array<T> // std::array provides both a tuple interface and a range interface.
  struct formatter<T>: std::formatter<std::string> {
    auto format(const T& tup, std::format_context& ctx) {
      if constexpr (std::tuple_size_v<T> == 0) {
        return formatter<std::string>::format("()", ctx);
      }
      else {
        return std::apply([&](auto&& head, auto&&... tail) {
          auto out = (std::format("({}", head) + ... + std::format(",{}", tail)) + ")";
          return formatter<std::string>::format(out, ctx);
        }, tup);
      }
    }
  };

  template <std::ranges::range T>
    requires mitama::display<std::ranges::range_value_t<T>>
          && mitama::not_string<T> // to avoid confilict of partial specializations
  struct formatter<T>: std::formatter<std::string> {
    auto format(const T& range, std::format_context& ctx) {
      auto inputs = range
                  | std::views::transform([](auto const& item) {
                      return std::format("{}", item);
                  });
      auto output = mitama::intercalate(",", inputs);
      return formatter<std::string>::format(std::format("[{}]", output), ctx);
    }
  };

  template <mitama::dictionary T>
    requires mitama::display<typename std::remove_cvref_t<T>::key_type>
          && mitama::display<typename std::remove_cvref_t<T>::mapped_type>
  struct formatter<T>: std::formatter<std::string> {
    auto format(const T& range, std::format_context& ctx) {
      auto inputs = range
                  | std::views::transform([](auto const& item) {
                      auto [key, val] = item;
                      return std::format("{}: {}", key, val);
                  });
      auto output = mitama::intercalate(",", inputs);
      return formatter<std::string>::format(std::format("{{{}}}", output), ctx);
    }
  };
}
