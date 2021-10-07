module;

#include <type_traits>
#include <concepts>
#include <functional>
#include <utility>

export module Mitama.Functional.Unwrap;
import Mitama.Maybe.def;
import Mitama.Result.def;

using namespace std;

namespace mitama {
  export inline constexpr auto unwrap
    = []<class T>(T&& x) -> decltype(auto)
      requires is_maybe<T> || is_result<T>
  {
    return std::forward<T>(x).unwrap();
  };
}
