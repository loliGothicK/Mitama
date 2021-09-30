module;
#include <array>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
export module Mitama.Data.Extensible.Record;
export import Mitama.Data.Extensible.StaticString;
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
      : Named( std::get<Indices>(args) )...
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

export namespace mitama:: inline where {
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
