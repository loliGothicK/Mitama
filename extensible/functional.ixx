module;
#include <utility>
export module Mitama.Functional.Extensible;

export namespace mitama {
  template <class ...Fn>
  struct overloaded : protected Fn... {
    template<class ...F>
    constexpr explicit overloaded(F&&... fn)
      : Fn(std::forward<F>(fn))...
    {}

    using Fn::operator()...;
  };

  template <class ...Fn>
  overloaded(Fn&&...) -> overloaded<Fn...>;
}
