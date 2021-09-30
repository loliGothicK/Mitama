module;
#include <boost/hana/functional/overload_linearly.hpp>
#include <string_view>
#include <variant>
export module Mitama.Data.Extensible.ADT;
export import Mitama.Data.Extensible.Named;
import Mitama.Data.Extensible.StaticString;
import Mitama.Data.Extensible.TypeList;
import Mitama.Functional.Extensible;
import Mitama.Utility.Extensible;

namespace mitama {
  template <auto, std::size_t, class...>
  struct index_of_impl;

  template <auto S, std::size_t I, class Head, class... Tail>
    requires (decltype(S)::value == Head::str)
  struct index_of_impl<S, I, Head, Tail...>
      : std::integral_constant<std::size_t, I>
  {};

  template <auto S, std::size_t I, class Head, class... Tail>
    requires (decltype(S)::value != Head::str)
  struct index_of_impl<S, I, Head, Tail...>
      : index_of_impl<S, I+1, Tail...>
  {};

  template <auto S, class... Named>
  inline constexpr std::size_t index_of = index_of_impl<S, 0, Named...>::value;
}

export namespace mitama {

  template <std::size_t I, class T>
  struct indexed {
    T value;
  };

  template <class, class...>
  class tagged_union;

  template <std::size_t ...Index, class... Named>
  class tagged_union<std::index_sequence<Index...>, Named...> {
    std::variant<indexed<Index, typename Named::value_type>...> storage;

    template <std::size_t I>
    static constexpr auto tag_v = default_v<list_element_t<I, type_list<decltype(Named::tag)...>>>;

  public:
    template <auto Tag>
    static constexpr std::size_t index_of = ::mitama::index_of<Tag, Named...>;

    template <auto Tag>
    using type_of = list_element_t<index_of<Tag>, type_list<typename Named::value_type...>>;


    template <auto S, class T>
    tagged_union(named<S, T> x)
      : storage{ std::in_place_index<index_of<S>>, x.value() }
    {}

    constexpr auto visit(auto&&... fn) const {
      return std::visit([&]<std::size_t I, class T>(indexed<I, T> x) -> void {
        std::invoke(
          boost::hana::overload_linearly(std::forward<decltype(fn)>(fn)...),
          default_v<named<tag_v<I>>>, x.value
        );
      }, this->storage);
    }
  };

  template <class ...Named>
  using union_type = tagged_union<std::index_sequence_for<Named...>, Named...>;

  template <class ...Fn>
  class match {
    std::tuple<Fn...> fn;
  public:
    template <class... F>
    constexpr explicit match(F&&... fs) : fn{ std::forward<F>(fs)... } {}

    template <class _, class ...Named>
    constexpr auto apply(tagged_union<_, Named...> const& tu) const {
      return std::apply([&](auto&&... fn) {
        return tu.visit(std::forward<decltype(fn)>(fn)...);
      }, fn);
    }
  };

  template <class ...Fn> match(Fn&&...) -> match<Fn...>;

  template <class _, class... Named, class... Fn>
  inline constexpr auto operator>>(tagged_union<_, Named...> tu, match<Fn...> match_expr) {
    return match_expr.apply(tu);
  }

  template <auto Case, class F>
  struct case_fn {
    F fn;

    template <auto S, class ...Args>
    constexpr auto operator()(named<S>, Args&&... args) const {
      return std::invoke(fn, std::forward<Args>(args)...);
    }
  };

  template <auto S>
  struct case_tag {
    static constexpr auto tag = S;

    constexpr auto operator--(int) const -> case_tag { return {}; }
  };

  template <auto S, class Fn>
  inline constexpr auto operator>(case_tag<S>, Fn&& fn) -> case_fn<S, Fn> {
    return case_fn<S, Fn>{ .fn = std::forward<Fn>(fn) };
  }
}

export namespace mitama:: inline literals:: inline match_literals{
  template <fixed_storage S>
  inline constexpr auto operator ""_then() noexcept {
    return case_tag<default_v<static_string<S>>>{};
  }
}
