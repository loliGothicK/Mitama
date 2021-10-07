module; // global module fragment

#include <sstream>
#include <string>
#include <string_view>
#include <format>
#include <boost/hana/functional/fix.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

export module Mitama.Result.IO;
import Mitama.Result.def;
import Mitama.Concepts.Formattable;

export namespace std {
  template <::mitama::formattable T, ::mitama::formattable E>
  struct formatter<::mitama::result<T, E>> : std::formatter<std::string> {
    auto format(const ::mitama::result<T, E>& res, ::std::format_context& ctx) {
      using namespace std::literals::string_literals;

      auto fmt = boost::hana::fix(boost::hana::overload_linearly(
        []<class T>(auto, T const& x) -> string
          requires requires { typename std::formatter<T>; }
          {
            if constexpr (std::same_as<T, std::monostate>) {
              return "()"s;
            }
            else if constexpr (std::convertible_to<std::string_view, T>) {
              return std::format("\"{}\"", x);
            }
            else {
              return std::format("{}", x);
            }
          },
        [](auto _fmt, mitama::dictionary auto const& x) -> string
          {
            if (ranges::empty(x)) return "{}"s;
            using ranges::begin, ranges::end;
            auto iter = begin(x);
            std::string str = "{"s + std::format("{}: {}", _fmt(std::get<0>(*iter)), _fmt(std::get<1>(*iter)));
            while (++iter != end(x)) {
              str += std::format(",{}: {}", _fmt(std::get<0>(*iter)), _fmt(std::get<1>(*iter)));
            }
            return str += "}";
          },
        [](auto _fmt, ranges::range auto const& x) -> string
          {
            if (ranges::empty(x)) return "{}"s;
            using ranges::begin, ranges::end;
            auto iter = begin(x);
            std::string str = "["s + _fmt(*iter);
            while (++iter != end(x)) {
              str += std::format(",{}", _fmt(*iter));
            }
            return str += "]";
          },
        [](auto _fmt, ranges::range auto const& x) -> string
          {
            if constexpr (tuple_size_v<decay_t<decltype(x)>> == 0) {
              return "()"s;
            }
            else {
              return apply(
                [_fmt](auto const& head, auto const&... tail) {
                  return "("s + _fmt(head) + ((("," + _fmt(tail))) + ...) + ")"s;
                }, x);
            }
          }
      ));
      return formatter<std::string>::format(
        bool(res)
          ? std::format("success({})", fmt(res.unwrap())) 
          : std::format("failure({})", fmt(res.unwrap_err())),
        ctx
      );
    }
  };
}

namespace mitama {
  export template <formattable T, formattable E>
  inline std::ostream& operator<<(std::ostream& os, result<T, E> const& res) {
    return os << std::format("{}", res);
  }
} //! namespace mitama
