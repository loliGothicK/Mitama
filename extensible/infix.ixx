module;
#include <type_traits>
#include <tuple>
#include <utility>
export module Mitama.Base.Functional.Infix;

export namespace mitama {
  template <std::size_t Precedence, class F, class... Partial>
  class infixed {
    mutable F fn;
    std::tuple<Partial...> partial;

    using tag = std::integral_constant<std::size_t, Precedence>;
  public:
    template <class Fn, class... Args>
    constexpr explicit infixed(tag, Fn&& fn, Args&&... args)
      : fn(std::forward<Fn>(fn))
      , partial(std::forward<Args>(args)...)
    {}

    template <class Fn, class... Args>
    constexpr explicit infixed(Fn&& fn, Args&&... args)
      : fn(std::forward<Fn>(fn))
      , partial(std::forward<Args>(args)...)
    {}

    template <class... Remaining>
    requires requires {
      typename std::invoke_result<F, Partial..., Remaining...>::type;
    }
    constexpr auto operator()(Remaining&&... remaining) const {
      return std::apply([&](auto&&... partial) {
        return std::invoke(fn,
          std::forward<Partial>(partial)...,
          std::forward<Remaining>(remaining)...
        );
      }, partial);
    }

    template <class... Remaining>
    constexpr auto operator()(Remaining&&... remaining) const {
      return std::apply([&](auto&&... partial) {
        return infixed{
          std::forward<F>(fn),
          std::forward<Partial>(partial)...,
          std::forward<Remaining>(remaining)...
        };
      }, partial);
    }

    template <class T>
    constexpr auto operator%(T&& x) const { return (*this)(std::forward<T>(x)); }
  };

  template <class F, class... Partial>
  infixed(F&&, Partial&&...)
    -> infixed<0, F, Partial...>;

  template <std::size_t Precedence, class F, class... Partial>
  infixed(std::integral_constant<std::size_t, Precedence>, F&&, Partial&&...)
    -> infixed<Precedence, F, Partial...>;

  template <std::size_t Precedence = 0>
  inline constexpr auto infix(auto&& fn, auto&&... partial) {
    return infixed{
      std::integral_constant<std::size_t, Precedence>{},
      std::forward<decltype(fn)>(fn),
      std::forward<decltype(partial)>(partial)...
    };
  };
}
