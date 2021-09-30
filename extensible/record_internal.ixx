module;
#include <array>
#include <algorithm>
#include <compare>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
export module Mitama.Data.Extensible.Record:Internal;
import Mitama.Utility.Extensible;
import Mitama.Data.Extensible.Named;
import Mitama.Data.Extensible.StaticString;
import Mitama.Data.Extensible.TypeList;

export namespace mitama {
  // This is power of C++20
  // FYI: 
  // https://stackoverflow.com/a/59567081
  // https://twitter.com/yaito3014/status/1442645605860347904

  // Named... :: class -> [usize; sizeof...(Named)]
  // Receive named and returns array of index that sorted state.
  template <class ...Named>
  constexpr auto sorted_indices = []<std::size_t ...Indices>(std::index_sequence<Indices...>) {
    using boxed_index = std::variant<std::integral_constant<std::size_t, Indices>...>;
    static_assert(!!sizeof...(Named));
    std::array arr{ boxed_index{std::in_place_index<Indices>}... };
    auto key = [](boxed_index boxed) {
      return std::visit([](auto i) {
        return std::remove_cvref_t<list_element_t<decltype(i)::value, type_list<Named...>>>::str;
        }, boxed);
    };
    auto cmp = [key](auto lhs, auto rhs) { return key(lhs) < key(rhs); };
    std::sort(arr.begin(), arr.end(), cmp);
    return arr;
  }(std::index_sequence_for<Named...>{});

  template <class, class...>
  struct sort;

  template <std::size_t ...I, class... Named>
  struct sort<std::index_sequence<I...>, Named...> {
    using type = type_list<list_element_t<sorted_indices<Named...>[I].index(), type_list<Named...>>...>;
  };

  // Returns sorted `Named...` packed into `type_list`.
  // [ Note:
  //    ```cpp
  //    using A = decltype("a"_ <= 1);
  //    using B = decltype("b"_ <= 2);
  //    using C = decltype("c"_ <= 3);
  // 
  //    using expected = type_list<A, B, C>;
  // 
  //    static_assert( std::same_as<expected, sorted<C, A, B>>); // OK
  //    ```
  //    -- end note ]
  template <class ...Named>
  using sorted = sort<std::index_sequence_for<Named...>, Named...>::type;

  template <class, class, class ...>
  struct difference;

  template <class Head, class ...Tail, class... Result, class ...ToRemove>
  struct difference<type_list<Head, Tail...>, type_list<Result...>, ToRemove...> {
    using type = std::conditional_t<
      ((Head::str == ToRemove::str) || ...),
      typename difference<type_list<Tail...>, type_list<Result...>, ToRemove...>::type,
      typename difference<type_list<Tail...>, type_list<Result..., Head>, ToRemove...>::type
    >;
  };

  template <class... Result, class ...ToRemove>
  struct difference<type_list<>, type_list<Result...>, ToRemove...> {
    using type = type_list<Result...>;
  };

  template <class List, static_string ...ToRemove>
  using erased = difference<List, type_list<>, named<ToRemove>...>::type;

  template <class First, class Second>
  concept equivalent_to = []<class ...L, class ...R>(type_list<L...>, type_list<R...>){
    if constexpr (sizeof...(L) != sizeof...(R)) return false;
    else {
      return ((L::str == R::str) && ...);
    }
  }(default_v<First>, default_v<Second>);
}
