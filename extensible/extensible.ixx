module;
#include <algorithm>
#include <array>
#include <compare>
#include <iterator>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
export module Mitama.Extensible;
export import Mitama.Extensible.Functional;
export import Mitama.Extensible.Utility;
export import Mitama.Data.Extensible.Named;
export import Mitama.Data.Extensible.StaticString;
export import Mitama.Data.Extensible.TypeList;

namespace mitama {
  // This is power of C++20
  // FYI: 
  // https://stackoverflow.com/a/59567081
  // https://twitter.com/yaito3014/status/1442645605860347904

  // Named... :: named_any -> [usize; sizeof...(Named)]
  // Receive named and returns array of index that sorted state.
  template <named_any ...Named>
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

  template <class, named_any...>
  struct sort;

  template <std::size_t ...I, named_any... Named>
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
  template <named_any ...Named>
  using sorted = sort<std::index_sequence_for<Named...>, Named...>::type;

  template <class, class, named_any ...>
  struct difference;

  template <named_any Head, named_any ...Tail, named_any... Result, named_any ...ToRemove>
  struct difference<type_list<Head, Tail...>, type_list<Result...>, ToRemove...> {
    using type = std::conditional_t<
      ((Head::str == ToRemove::str) || ...),
      typename difference<type_list<Tail...>, type_list<Result...>, ToRemove...>::type,
      typename difference<type_list<Tail...>, type_list<Result..., Head>, ToRemove...>::type
    >;
  };

  template <named_any... Result, named_any ...ToRemove>
  struct difference<type_list<>, type_list<Result...>, ToRemove...> {
    using type = type_list<Result...>;
  };

  template <class List, static_string ...ToRemove>
  using erased = difference<List, type_list<>, named<ToRemove>...>::type;
}

// Concepts for Extensible Records
export namespace mitama {
  template <class ...Named>
  concept distinct = [] {
    if constexpr (sizeof...(Named) < 2) return true;
    else {
      std::array keys{ Named::str... };
      std::sort(keys.begin(), keys.end());
      return std::adjacent_find(keys.begin(), keys.end()) == keys.end();
    }
  }();

  template <class ...Named>
  concept is_sorted = [] {
    if constexpr (sizeof...(Named) < 2) return true;
    else {
      std::array keys{ Named::str... };
      return std::is_sorted(keys.begin(), keys.end());
    }
  }();
}

// Extensible Records
export namespace mitama {
  template <class = type_list<>>
  class record;

  template <named_any ...Named>
    requires distinct<Named...> 
          && is_sorted<Named...>
  class record<type_list<Named...>>
      : protected Named...
  {
    template <std::size_t ...Indices, class ...Args>
    constexpr explicit record(std::index_sequence<Indices...>, std::tuple<Args...> args)
      noexcept((noexcept(Named{ std::get<Indices>(args) }) && ...))
      : Named{ std::get<Indices>(args) }...
    {}
  public:
    static constexpr std::array tags = { Named::str... };

    template <named_any ...Args>
    constexpr record(Args&&... args)
      : record{
          std::index_sequence_for<Args...>{},
          [t = std::forward_as_tuple(std::forward<Args>(args)...)]
          <std::size_t ...Indices>(std::index_sequence<Indices...>) {
            return std::forward_as_tuple(
              std::get<sorted_indices<Args...>[Indices].index()>(t)...);
          }(std::index_sequence_for<Args...>{})
        }
    {}
    
    using Named::operator[]...;

    template <named_any ...New>
    using spread = record< sorted<Named..., New...> >;

    template <static_string ...Keys>
    using shrink = record< erased<type_list<Named...>, Keys...> >;

    template <named_any Ex>
    constexpr auto operator|(Ex ex) const {
      return spread<Ex>(Named::clone()..., ex);
    }
  };

  template <named_any ...Named>
  record(Named...) -> record<sorted<Named...>>;

  template <>
  class record<type_list<>> {};

  inline constexpr record<> empty{};

  template <named_any Named>
  inline constexpr auto operator|(record<>, Named named) {
    return record{ named };
  }

  template <class ...Named>
  using record_type = record<sorted<Named...>>;
}

export namespace mitama:: inline where{
  template <class Record, static_string ...Required>
  concept has = [] {
    auto tags = Record::tags;
    return (std::binary_search(tags.cbegin(), tags.cend(), decltype(Required)::value) && ...);
  }();

  template <class Record>
  concept records = [] {
    return overloaded{
      [](...) { return false; },
      []<class ..._>(std::type_identity<record<type_list<_...>>>) { return true; }
    }(std::type_identity<std::remove_cvref_t<Record>>());
  }();
}
