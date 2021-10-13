module;
#include <type_traits>
#include <utility>
export module Mitama.Base.Control.Applicative;
export import Mitama.Base.Control.Functor;
import Mitama.Base.Functional.Infix;

export namespace mitama {
  // Applicative Concept
  //    pure   :: a -> f a
  //    (<*>)  :: f (a -> b) -> f a -> f b
  //    liftA2 :: (a -> b -> c) -> f a -> f b -> f c
  //    (*>)   :: f a -> f b -> f b
  //    (<*)   :: f a -> f b -> f a
  //
  // Identity
  //    pure id <*> v = v
  // Composition
  //    pure (.) <*> u <*> v <*> w = u <*> (v <*> w)
  // Homomorphism
  //    pure f <*> pure x = pure (f x)
  // Interchange
  //    u <*> pure y = pure ($ y) <*> u
  //

  template <functor Applicative>
  struct applicative_traits {
    // seq_apply :: f (a -> b) -> f a -> f b
    static constexpr auto seq_apply = infix([](auto&& ab, auto&& a) {
      return std::remove_cvref_t<Applicative>::seq_apply(
        std::forward<decltype(ab)>(ab),
        std::forward<decltype(a)>(a)
      );
    });

    // liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    static constexpr auto liftA2 = infix([](auto&& fn, auto&& ap1, auto&& ap2) {
      return std::remove_cvref_t<Applicative>::liftA2(
        std::forward<decltype(fn)>(fn),
        std::forward<decltype(ap1)>(ap1),
        std::forward<decltype(ap2)>(ap2)
      );
    });

    // (*>) :: f a -> f b -> f b
    // (<*) ::f a->f b->f a
    template <std::size_t I>
    static constexpr auto discard = infix([](auto&& fst, auto&& snd) {
      if constexpr (I == 0) {
        return std::forward<decltype(snd)>(snd);
      }
      else if constexpr (I == 1) {
        return std::forward<decltype(fst)>(fst);
      }
      else {
        static_assert(I < 2);
      }
    });

  };

  template <class Ap>
  concept applicative = functor<Ap> && requires {
    applicative_traits<std::remove_cvref_t<Ap>>::seq_apply;
    applicative_traits<std::remove_cvref_t<Ap>>::liftA2;
    applicative_traits<std::remove_cvref_t<Ap>>::template discard<0>;
    applicative_traits<std::remove_cvref_t<Ap>>::template discard<1>;
  };

  template <template <class...> class Applicative>
  struct lift {
    template <class T>
    using pure = Applicative<T>;

    template <class T>
    static constexpr auto from(T&& a) -> applicative decltype(auto)
    { return pure(std::forward<T>(a)); }
  };

  template <template <class...> class Applicative>
  inline constexpr auto pure = infix([]<class T>(T&& a) {
    return lift<Applicative>::from(std::forward<T>(a));
  });

}
