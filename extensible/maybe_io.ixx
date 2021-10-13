module; // global module fragment

#include <sstream>
#include <string>
#include <string_view>
#include <format>
#include <boost/hana/functional/fix.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

export module Mitama.Data.Maybe.IO;
import Mitama.Data.Maybe.def;
import Mitama.Base.Data.Display;

export namespace std {
  template <mitama::display T>
  struct formatter<::mitama::maybe<T>> : std::formatter<std::string> {
    auto format(const ::mitama::maybe<T>& may, ::std::format_context& ctx) {
      return formatter<std::string>::format(
        bool(may)
          ? std::format("just({})", fmt(may.unwrap())) 
          : "nothing"s,
        ctx
      );
    }
  };
}

namespace mitama {
  export template <display T>
  inline std::ostream& operator<<(std::ostream& os, maybe<T> const& may) {
    return os << std::format("{}", may);
  }
} //! namespace mitama
