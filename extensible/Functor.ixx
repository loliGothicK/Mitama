module;
#include <type_traits>
#include <tuple>
#include <utility>
export module Mitama.Base.Control.Functor;
import Mitama.Base.Functional.Infix;

export namespace mitama {
  // Functor Concept
  //    fmap :: (a -> b) -> f a -> f b
  // 
  // Identity
  //    fmap id == id
  // Composition
  //    fmap (f . g) == fmap f . fmap g
  //

  template <class Functor>
  struct functor_traits {
    static constexpr auto fmap = infix([](auto&& map, auto&& functor) {
      return std::remove_cvref_t<Functor>::fmap(
        std::forward<decltype(map)>(map),
        std::forward<decltype(functor)>(functor)
      );
    });
  };

  inline constexpr auto fmap = infix([]<class F>(auto&& map, F&& functor) {
    return std::invoke(
      functor_traits<std::remove_cvref_t<F>>::fmap,
      std::forward<decltype(map)>(map),
      std::forward<F>(functor)
    );
  });

  template <class F>
  concept functor = requires {
    functor_traits<std::remove_cvref_t<F>>::fmap;
  };
}
