module;
#include <array>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
export import Mitama.Data.Extensible.Named;
export import Mitama.Data.Extensible.StaticString;
export module Mitama.Data.Extensible.Record;
import Mitama.Data.Extensible.TypeList;
import Mitama.Functional.Extensible;
import Mitama.Utility.Extensible;
import :Internal;

export namespace mitama:: inline where {
  template <class T, static_string Tag>
  concept named_as = [] {
    return overloaded{
      [](...) { return false; },
      []<class _>(std::type_identity<named<Tag, _>>) { return true; }
    }(std::type_identity<std::remove_cvref_t<std::remove_cvref_t<T>>>());
  }();

  template <class T>
  concept named_any = [] {
    return overloaded{
      [](...) { return false; },
      []<auto Any, class _>(std::type_identity<named<Any, _>>) { return true; }
    }(std::type_identity<std::remove_cvref_t<std::remove_cvref_t<T>>>());
  }();
}

// Concepts for Extensible Records
export namespace mitama:: inline where {
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
  template <class = type_list<>, class = type_list<>>
  class record;

  template <named_any ...Fields>
    requires distinct<Fields...>
  class record<type_list<Fields...>>
      : protected Fields...
  {
    template <named_any... From>
    struct FROM: protected From... {
      constexpr FROM(From... from): From(from)... {}
      using From::operator[]...;
    };
    template <named_any ...Args>
    constexpr record(FROM<Args...> table)
      : Fields( table[Fields::tag] )...
    {}

    using fields = type_list<Fields...>;
    using sorted = sorted<Fields...>;

  public:
    template <named_any ...Args>
    constexpr record(Args... args): record(FROM<Args...>(std::forward<Args>(args)...)) {}
    
    template <static_string Key>
    using typeof = decltype(std::declval<FROM<Fields...>>()[Key]);

    using Fields::operator[]...;

    // TODO: impl `merge` and use `merge<type_list<Sorted>, sorted<New...>>` instead of `sorted<Sorted..., New...>`
    template <named_any ...New>
    using spread = record< type_list<Fields..., New...> >;

    template <static_string ...Keys>
    using shrink = record< erased<type_list<Fields...>, Keys...> >;

    template <named_any Ex>
    constexpr auto operator+=(Ex ex) const {
      return spread<Ex>(Fields::clone()..., ex);
    }

    template <named_any ...Ex>
    constexpr auto operator+=(std::tuple<Ex...> ex) const {
      return std::apply([](auto&& ...ex) {
        return spread<std::remove_cvref_t<Ex>...>{
          Fields::clone()...,
          std::forward<decltype(ex)>(ex)...
        };
      }, ex);
    }
  };

  template <named_any ...Named>
  record(Named...) -> record<type_list<Named...>>;

  template <>
  class record<type_list<>> {};

  inline constexpr record<> empty{};

  template <named_any Named>
  inline constexpr auto operator+=(record<>, Named named) {
    return record{ named };
  }

  template <named_any ...Ex>
  constexpr auto operator+=(record<>, std::tuple<Ex...> ex) {
    return std::apply([](auto&& ...ex) {
      return record<type_list<std::remove_cvref_t<Ex>...>>{ std::forward<decltype(ex)>(ex)... };
    }, ex);
  }

  template <named_any ...Named>
  using record_type = record<type_list<Named...>>;

  template <static_string ...S>
  inline constexpr auto select = [](auto&& rec) {
    return std::tuple(rec[S]...);
  };
}

export namespace mitama:: inline where {
  template <class Record, static_string ...Required>
  concept has = []<named_any... Fields>(std::type_identity<record<type_list<Fields...>>>) {
    return []<named_any... Sorted>(type_list<Sorted...>){
      std::array tags = { Sorted::str... };
      return (std::binary_search(tags.cbegin(), tags.cend(), decltype(Required)::value) && ...);
    }(sorted<Fields...>());
  }(std::type_identity<Record>());

  template <class Record>
  concept records = [] {
    return overloaded{
      [](...) { return false; },
      []<class ..._>(std::type_identity<record<type_list<_...>>>) { return true; }
    }(std::type_identity<std::remove_cvref_t<Record>>());
  }();
}
