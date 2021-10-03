module;
#include <boost/hana/functional/overload_linearly.hpp>
#include <string_view>
#include <variant>
export module Mitama.Data.Extensible.ADT;
export import Mitama.Data.Extensible.Named;
import Mitama.Data.Extensible.StaticString;
import Mitama.Data.Extensible.TypeList;
import Mitama.Concepts.Extensible;
import Mitama.Functional.Extensible;
import Mitama.Utility.Extensible;

// index reverse computer
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

  // value wrapper
  template <std::size_t I, class T>
  struct indexed {
    T value;
  };

  template <class, class...>
  class tagged_union;

  // tagged union
  template <std::size_t ...Index, class... Named>
  class tagged_union<std::index_sequence<Index...>, Named...> {
    std::variant<indexed<Index, typename Named::value_type>...> storage;

    template <std::size_t I>
    static constexpr auto tag_v = default_v<list_element_t<I, type_list<decltype(Named::tag)...>>>;
    struct deduction {
      constexpr deduction(auto&&...) noexcept {}
    };
  public:
    template <auto Tag>
    static constexpr std::size_t index_of = ::mitama::index_of<Tag, Named...>;

    template <auto Tag>
    using type_of = list_element_t<index_of<Tag>, type_list<typename Named::value_type...>>;

    template <auto S, class T>
    tagged_union(named<S, T> x)
      : storage{ std::in_place_index<index_of<S>>, x.value() }
    {}

    template <class Ret = deduction, class... Fn>
      requires (std::is_invocable_r_v<Ret, Fn, named<std::decay_t<Fn>::tag>, type_of<std::decay_t<Fn>::tag>> && ...)
    constexpr auto inspect(Fn&&... fn) const {
      return std::visit([&]<std::size_t I, class T>(indexed<I, T> x) {
        if constexpr (std::same_as<Ret, deduction>) {
          using type = std::common_reference_t<
            std::invoke_result_t< Fn, named<std::decay_t<Fn>::tag>, type_of<std::decay_t<Fn>::tag> >...
          >;
          return static_cast<type>(std::invoke(
            boost::hana::overload_linearly(std::forward<decltype(fn)>(fn)...),
            default_v<named<tag_v<I>>>, x.value
          ));
        }
        else {
          return static_cast<Ret>(std::invoke(
            boost::hana::overload_linearly(std::forward<decltype(fn)>(fn)...),
            default_v<named<tag_v<I>>>, x.value
          ));
        }
      }, this->storage);
    }
  };

  template <class ...Named>
  using union_type = tagged_union<std::index_sequence_for<Named...>, Named...>;
}

export namespace mitama {
  template <class Ret, class TaggedUnion>
  struct inspector {
    TaggedUnion tu;

    template <class... Fn>
    auto inspect(Fn&&... fn) {
      if constexpr (std::is_void_v<Ret>) {
        return tu.inspect(std::forward<Fn>(fn)...);
      }
      else {
        return tu.inspect<Ret>(std::forward<Fn>(fn)...);
      }
    }
  };
  
  template <class T = void>
  struct match_ {};

  inline constexpr match_<> match{};

  template <class Ret, class T>
  inline constexpr auto operator|(match_<Ret>, T&& target) {
    return inspector<Ret, T>{ .tu = std::forward<T>(target) };
  }

  template <class ...Fn>
  class with {
    std::tuple<Fn...> fn;
  public:
    template <class... F>
    constexpr explicit with(F&&... fs) : fn{ std::forward<F>(fs)... } {}

    constexpr auto apply(auto&& inspector) const {
      return std::apply([&](auto&&... fn) {
        return inspector.inspect(std::forward<decltype(fn)>(fn)...);
      }, fn);
    }
  };

  template <class ...Fn> with(Fn&&...) -> with<Fn...>;

  inline constexpr auto operator|(auto&& inspector, kind<of<with>> auto&& with) {
    return with.apply(inspector);
  }

  template <auto Case, class F>
  struct case_fn {
    static constexpr auto tag = Case;
    F fn;

    template <auto S, class ...Args> requires (decltype(Case)::value == named<S>::str)
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

export namespace mitama:: inline literals:: inline with_literals{
  template <fixed_storage S>
  inline constexpr auto operator ""_then() noexcept {
    return case_tag<default_v<static_string<S>>>{};
  }
}
