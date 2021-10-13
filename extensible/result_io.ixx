module; // global module fragment

#include <sstream>
#include <string>
#include <string_view>
#include <format>
#include <boost/hana/functional/fix.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

export module Mitama.Data.Result.IO;
import Mitama.Data.Result.def;
import Mitama.Base.Data.Display;

export namespace std {
  template <::mitama::display T, ::mitama::display E>
  struct formatter<::mitama::result<T, E>> : std::formatter<std::string> {
    auto format(const ::mitama::result<T, E>& res, ::std::format_context& ctx) {
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
  export template <display T, display E>
  inline std::ostream& operator<<(std::ostream& os, result<T, E> const& res) {
    return os << std::format("{}", res);
  }
} //! namespace mitama
