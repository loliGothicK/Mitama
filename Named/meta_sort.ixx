module;
#include <type_traits>
#include <tuple>
export module meta_sort;

export namespace mitama {
  template <class ...> struct type_list{};

  template <std::size_t, class>
  struct list_element {};

  template <std::size_t I, class Head, class ...Tail> requires (I == 0)
  struct list_element<I, type_list<Head, Tail...>>
    : std::type_identity<Head>
  {};
  template <std::size_t I, class Head, class ...Tail> requires (I != 0)
  struct list_element<I, type_list<Head, Tail...>>
    : std::type_identity<typename list_element<I - 1, type_list<Tail...>>::type>
  {};

  template <std::size_t I, class T>
  using list_element_t = list_element<I, T>::type;
}


namespace mitama {
  template<std::size_t N, std::size_t... Seq>
  consteval std::index_sequence<(N + Seq) ...>
    add(std::index_sequence<Seq...>) { return {}; }

  template<std::size_t Min, std::size_t Upper>
  using make_index_range = decltype(add<Min>(std::make_index_sequence<Upper - Min>()));
}

namespace mitama {
  template <auto, template<class> class, class, class, class>
  struct merge_impl;

  template <auto Cmp, template<class> class Proj, class ...Lefts, class ...Sorted>
  struct merge_impl<Cmp, Proj, type_list<Lefts...>, type_list<>, type_list<Sorted...>>
  {
    using type = type_list<Sorted..., Lefts...>;
  };

  template <auto Cmp, template<class> class Proj, class ...Rights, class ...Sorted>
  struct merge_impl<Cmp, Proj, type_list<>, type_list<Rights...>, type_list<Sorted...>>
  {
    using type = type_list<Sorted..., Rights...>;
  };

  template <auto Cmp, template<class> class Proj, class L, class ...Lefts, class R, class ...Rights, class ...Sorted>
  struct merge_impl<Cmp, Proj, type_list<L, Lefts...>, type_list<R, Rights...>, type_list<Sorted...>>
  {
    using type = std::conditional_t<
      static_cast<bool>(Cmp(
        std::type_identity<typename Proj<L>::type>{},
        std::type_identity<typename Proj<R>::type>{}
      )),
      typename merge_impl<
        Cmp, Proj,
        type_list<Lefts...>,
        type_list<R, Rights...>,
        type_list<Sorted..., L>
      >::type,
      typename merge_impl<
        Cmp, Proj,
        type_list<L, Lefts...>,
        type_list<Rights...>,
        type_list<Sorted..., R>
      >::type
    >;
  };

  template <auto, template<class> class, class, class>
  struct merge;

  template <auto Cmp, template <class> class Proj, class... Lefts, class... Rights>
  struct merge<Cmp, Proj, type_list<Lefts...>, type_list<Rights...>>
  {
    using type = merge_impl<
      Cmp, Proj,
      type_list<Lefts...>,
      type_list<Rights...>,
      type_list<>
    >::type;
  };

  template <auto Cmp, template <class> class Proj, class L, class R>
  using merged = merge<Cmp, Proj, L, R>::type;

  template <class, class, class>
  struct split_impl;

  template <std::size_t ...L, std::size_t ...R, class ...Types>
  class split_impl<std::index_sequence<L...>, std::index_sequence<R...>, type_list<Types...>>
  {
    using list = type_list<Types...>;
    template <size_t N, class List>
    using lefts = list_element_t<N, List>;
    template <size_t N, class List>
    using rights = list_element_t<N, List>;
  public:
    using type = type_list<
        type_list<lefts<L, list>...>,
        type_list<rights<R, list>...>
      >;
  };

  template <class>
  struct split;

  template <class ...Types>
  struct split<type_list<Types...>>
  {
    using type = split_impl<
      make_index_range<0, sizeof...(Types) / 2>,
      make_index_range<sizeof...(Types) / 2, sizeof...(Types)>,
      type_list<Types...>
    >::type;
  };

  template <class T>
  using splitted = split<T>::type;

  template <auto, template <class> class, class>
  class merge_sort_impl;

  template <auto Cmp, template <class> class Proj, class T>
  class merge_sort_impl<Cmp, Proj, type_list<type_list<>, type_list<T>>>
  {
  public:
    using type = type_list<T>;
  };

  template <auto Cmp, template <class> class Proj, class L, class R>
  class merge_sort_impl<Cmp, Proj, type_list<type_list<L>, type_list<R>>>
  {
  public:
    using type = merged<Cmp, Proj, type_list<L>, type_list<R>>;
  };

  template <auto Cmp, template <class> class Proj, class ...Lefts, class ...Rights>
  class merge_sort_impl<Cmp, Proj, type_list<type_list<Lefts...>, type_list<Rights...>>>
  {
    using left  = merge_sort_impl<Cmp, Proj, splitted<type_list<Lefts...>>>::type;
    using right = merge_sort_impl<Cmp, Proj, splitted<type_list<Rights...>>>::type;
  public:
    using type = merged<Cmp, Proj, left, right>;
  };

  template <template<class...> class, class T> struct hull;

  template <template<class...> class TT, class ...Types>
  struct hull<TT, type_list<Types...>>
    : std::type_identity<TT<Types...>>
  {};

  template <template<class...> class, auto, template <class> class, class>
  struct merge_sort_facade;

  template <template<class...> class TT, auto Cmp, template <class> class Proj, class ...Types>
  class merge_sort_facade<TT, Cmp, Proj, type_list<Types...>>
  {
    using sorted_list
      = merge_sort_impl<
        Cmp,
        Proj,
        typename split<
          type_list<Types...>
        >::type
      >::type;
    public:
      using type = hull<TT, sorted_list>::type;
  };

  template <class, auto, template <class> class> struct merge_sort;

  template <template<class...> class TT, class... Types, auto Cmp, template <class> class Proj>
  requires (requires { typename Proj<Types>::type; } && ...)
  struct merge_sort<TT<Types...>, Cmp, Proj>
    : merge_sort_facade<TT, Cmp, Proj, type_list<Types...>>
  {};
}

export namespace mitama {
  template <
    class List, // TT<Ts...>
    auto Cmp = std::less<>{},
    template <class> class Proj = std::type_identity
  >
  using sorted = merge_sort<List, Cmp, Proj>::type;
}
