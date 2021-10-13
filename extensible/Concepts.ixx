module;
#include <type_traits>
#include <ranges>
export module Mitama.Base.Concepts.DataKind;

namespace mitama {
  template <template <class...> class, class>
  struct of_impl: std::false_type {};
  template <template <class...> class Expected, class... _>
  struct of_impl<Expected, Expected<_...>>: std::true_type {};
}

export namespace mitama:: inline where {
  template <template <class...> class Expected>
  struct of {
    template <class T>
    static constexpr bool value = of_impl<Expected, std::remove_cvref_t<T>>::value;
  };

  template <class T, class Of>
  concept kind = Of:: template value<T>;
}

export namespace mitama:: inline where{
  template <class R, class E>
  concept range_of = std::ranges::range<R>
                  && std::convertible_to<std::ranges::range_value_t<R>, E>;
}
