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
import Mitama.Concepts.Extensible;
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

namespace mitama {
  template <named_any... Rows>
  requires distinct<Rows...>
    class record;

  template <class T>
  struct make_record_impl;

  template <named_any... Rows>
  struct make_record_impl<type_list<Rows...>>
    : std::type_identity<record<Rows...>>
  {};

  template <class List>
  using make_record = make_record_impl<List>::type;
}

// Extensible Records
export namespace mitama {
  template <named_any ...Rows>
    requires distinct<Rows...>
  class record;

  template <class Record>
  class shrink {
    Record record;
  public:
    template <class R>
    constexpr shrink(R&& r) : record(std::forward<R>(r)) {}

    template <auto S>
    constexpr decltype(auto) operator[](static_string<S> tag) const {
      return record[tag];
    }
  };

  template <kind<of<record>> Record>
  shrink(Record&&) -> shrink<Record>;

  template <named_any ...Rows>
    requires distinct<Rows...>
  class record
      : protected Rows...
  {
    template <named_any... From>
    struct FROM: protected From... {
      constexpr FROM(From... from): From(from)... {}
      using From::operator[]...;
    };
    template <named_any ...Args>
    constexpr record(FROM<Args...> table)
      : Rows( table[Rows::tag] )...
    {}

  public:
    template <named_any ...Args>
    constexpr record(Args... args)
      : record(FROM<Args...>(std::forward<Args>(args)...))
    {}
    
    template <superset_of<record> Record>
    constexpr record(shrink<Record> other)
      : Rows(other[Rows::tag])...
    {}

    template <static_string Key>
    using typeof = decltype(std::declval<FROM<Rows...>>()[Key]);

    using Rows::operator[]...;

    template <named_any ...New>
    using spread = record<Rows..., New...>;

    template <static_string ...Keys>
    using shrink = make_record<erased<type_list<Rows...>, Keys...>>;

    template <named_any Ex>
    constexpr auto operator+=(Ex ex) const {
      return spread<Ex>(Rows::clone()..., ex);
    }

    template <named_any ...Ex>
    constexpr auto operator+=(std::tuple<Ex...> ex) const {
      return std::apply([](auto&& ...ex) {
        return spread<std::remove_cvref_t<Ex>...>{
          Rows::clone()...,
          std::forward<decltype(ex)>(ex)...
        };
      }, ex);
    }
  };

  template <named_any ...Rows>
  record(Rows...) -> record<Rows...>;

  template <>
  class record<> {};

  inline constexpr record<> empty{};

  template <named_any Row>
  inline constexpr auto operator+=(record<>, Row named) {
    return record{ named };
  }

  template <named_any ...Ex>
  constexpr auto operator+=(record<>, std::tuple<Ex...> ex) {
    return std::apply([](auto&& ...ex) {
      return record<std::remove_cvref_t<Ex>...>{ std::forward<decltype(ex)>(ex)... };
    }, ex);
  }

  template <static_string ...S>
  inline constexpr auto select = [](auto&& rec) {
    return std::tuple(rec[S]...);
  };
}

export namespace mitama:: inline where {
  template <class Record, static_string ...Required>
  concept has = []<named_any... Rows>(std::type_identity<record<Rows...>>) {
    return []<named_any... Sorted>(type_list<Sorted...>){
      std::array tags = { Sorted::str... };
      return (std::binary_search(tags.cbegin(), tags.cend(), decltype(Required)::value) && ...);
    }(sorted<Rows...>());
  }(std::type_identity<Record>());
}
