module;
#include <type_traits>
#include <utility>
#include <optional>
export module Mitama.Base.Control.Monad;
export import Mitama.Base.Control.Applicative;
import Mitama.Base.Functional.Infix;

export namespace mitama {
  // Functor Concept
  //    (>>=) :: forall a b. m a -> (a -> m b) -> m b
  // 
  // Left identity
  //    return a >>= k = k a
  // Right identity
  //    m >>= return = m
  // Associativity
  //    m >>= (\x -> k x >>= h) = (m >>= k) >>= h
  //

  template <applicative Monad>
  struct monad_traits {
    static constexpr auto bind = infix([](auto&& map, auto&& functor) {
      return std::remove_cvref_t<Monad>::bind(
        std::forward<decltype(map)>(map),
        std::forward<decltype(functor)>(functor)
      );
    });
  };

  template <class Monad>
  concept monad = applicative<Monad> && requires {
    monad_traits<std::remove_cvref_t<Monad>>::bind;
  };

  inline constexpr auto bind = infix([]<class Monad>(Monad && ma, auto && proj) {
    return std::invoke(
      monad_traits<std::remove_cvref_t<Monad>>::bind,
      std::forward<decltype(ma)>(ma),
      std::forward<decltype(proj)>(proj)
    );
  });
}

namespace mitama {
  template <template <class...> class Monadic>
  struct returns {
    template <class T>
    using inject = Monadic<T>;

    template <class T>
    static constexpr auto from(T&& a)->monad decltype(auto)
    { return inject(std::forward<T>(a)); }
  };
}

export namespace mitama {
  template <template <class...> class Monadic>
  inline constexpr auto inject = infix([](auto&& a) {
    return returns<Monadic>::from(std::forward<decltype(a)>(a));
  });
}
