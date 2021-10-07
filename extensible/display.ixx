module;

#include <iostream>
#include <format>

export module Mitama.Display;

namespace mitama {
  export template <class T, class CharT = char>
    requires requires { typename std::formatter<T, CharT>; }
  struct display {
    T value;

    friend std::ostream& operator<<(std::ostream& os, display const& disp) {
      return os << std::format("{}", disp.value);
    }
  };
} //! namespace mitama

export namespace std {
  template <class T>
  struct formatter<::mitama::display<T>>: std::formatter<std::string> {
    auto format(const ::mitama::display<T>& disp, std::format_context& ctx) {
      return formatter<std::string>::format(disp.value, ctx);
    }
  };
}
