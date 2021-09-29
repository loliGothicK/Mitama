module;
#include <tuple>
#include <utility>
export module Mitama.Data.Extensible.Into;

export namespace mitama {
  // placeholder type for in place construction
  template <auto Tag, class ...Args>
  struct into_ {
    std::tuple<Args...> args;
  };

  template <auto _>
  inline constexpr auto into = []<class ...Args>(Args&&... args) {
    return into_<_, Args...>{
      .args = std::forward_as_tuple(std::forward<Args>(args)...)
    };
  };
}
