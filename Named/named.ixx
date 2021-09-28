module;
#include <utility>
#include <string_view>
#include <type_traits>
#include <compare>
#include <array>
#include <algorithm>
#include <variant>
#include <tuple>
#include <ranges>
#include <vector>
export module named;

namespace mitama {
  template <std::default_initializable T>
  inline constexpr T default_v{};
}

/// <summary>
/// `static_string`: Phantom-type for `named<Tag, Ty>`.
/// </summary>
export namespace mitama {
  template<std::size_t N>
  struct fixed_storage {
    static constexpr std::size_t size = N;

    constexpr fixed_storage(char const (&s)[N])
      : fixed_storage(s, std::make_index_sequence<N>{}) {}
    template<std::size_t ...Indices>
    constexpr fixed_storage(char const (&s)[N], std::index_sequence<Indices...>)
      : s{ s[Indices]... } {}

    char const s[N];
  };

  template<auto S>
  struct static_string {
    static constexpr auto storage = S;
    static constexpr std::string_view const value{ storage.s, decltype(storage)::size };

    //! auto parameters
    //! consistent comparisons
    template <auto T>
    constexpr std::strong_ordering operator<=>(static_string<T>) const noexcept {
      return static_string<S>::value <=> static_string<T>::value;
    }
  };

  // tag literal
  //! auto parameters
  namespace literals:: inline named_literals {
    template<fixed_storage S>
    inline constexpr static_string<S> operator""_() {
      return {};
    }
  }
}

// concept helper forwad declarations
namespace mitama::mitamagic {
  template <static_string, class>
  struct is_named_as : std::false_type {};

  template <class>
  struct is_named_any : std::false_type {};
}

// concepts
export namespace mitama {
  template <class T, static_string Tag>
  concept named_as = mitamagic::is_named_as<Tag, std::remove_cvref_t<T>>::value;

  template <class T>
  concept named_any = mitamagic::is_named_any<std::remove_cvref_t<T>>::value;
}

namespace mitama {
  // placeholder type for emplace construction
  template <static_string Tag, class ...Args>
  struct into {
    std::tuple<Args...> args;
  };

  // primary
  // owned storage
  template <class T>
  class named_storage {
    T value;
  public:
    named_storage() = delete;
    constexpr named_storage(named_storage const&) = default;
    constexpr named_storage(named_storage&&) = default;
    constexpr named_storage& operator=(named_storage const&) = default;
    constexpr named_storage& operator=(named_storage&&) = default;

    template <class ...Args>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : value( std::forward<Args>(args)... )
    {}

    template <class ...Args>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(std::tuple<Args...> into)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage{ into, std::index_sequence_for<Args...>{} }
    {}

    template <class ...Args, std::size_t ...Indices>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(std::tuple<Args...> into, std::index_sequence<Indices...>)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage{ std::get<Indices>(into)...}
    {}

    decltype(auto) deref() &      { return value; }
    decltype(auto) deref() const& { return value; }
    
    decltype(auto) indirect() &      { return std::addressof(value); }
    decltype(auto) indirect() const& { return std::addressof(value); }
  };

  // specialization
  // borrowed storage
  template <class T>
  class named_storage<T&> {
    std::reference_wrapper<T> ref;
  public:
    named_storage() = delete;
    constexpr named_storage(named_storage const&) = default;
    constexpr named_storage(named_storage&&) = default;
    constexpr named_storage& operator=(named_storage const&) = default;
    constexpr named_storage& operator=(named_storage&&) = default;

    template <class U>
      requires std::constructible_from<std::reference_wrapper<T>, U>
    constexpr named_storage(U&& from)
      noexcept(std::is_nothrow_constructible_v<std::reference_wrapper<T>, U>)
      : ref{ from }
    {}

    decltype(auto) deref() &      { return ref.get(); }
    decltype(auto) deref() const& { return ref.get(); }
    
    decltype(auto) indirect() &      { return std::addressof(ref.get()); }
    decltype(auto) indirect() const& { return std::addressof(ref.get()); }
  };
}

export namespace mitama {
  // `named`: opaque-type that strict-typed via phantom-type `Tag`.
  template <static_string Tag, class T = std::void_t<>>
  class named: named_storage<T> {
    using storage = named_storage<T>;
  public:
    static constexpr std::string_view str = decltype(Tag)::value;
    static constexpr static_string tag = Tag;

    constexpr named() = delete;
    constexpr named(named const&) = default;
    constexpr named(named&&) = default;
    constexpr named& operator=(named const&) = default;
    constexpr named& operator=(named&&) = default;

    template <class U> requires std::constructible_from<T, U>
    constexpr explicit(!std::is_convertible_v<U, T>)
    named(U&& from)
      noexcept(std::is_nothrow_constructible_v<T, U>)
      : named_storage<T>{ std::forward<U>(from) }
    {}

    template <class ...Args> requires std::constructible_from<T, Args...>
    constexpr named(into<Tag, Args...> into)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage<T>{ into.args }
    {}

    decltype(auto) value() &      { return storage::deref(); }
    decltype(auto) value() const& { return storage::deref(); }

    // dereference
    auto operator*() -> T {
      return storage::deref();
    }

    // indirections
    auto operator->() & -> std::decay_t<T>* {
      return storage::indirect();
    }
    auto operator->() const& -> std::decay_t<T> const* {
      return storage::indirect();
    }

  private:
    // friend declaration for visibility of `oerator[]`
    template <class>
    friend class record;

    // for records
    constexpr auto operator[](decltype(Tag)) const noexcept -> T {
      return storage::deref();
    }
  };

}

export namespace mitama::literals:: inline named_literals {
  template <auto S, class T>
  constexpr auto operator<=(static_string<S>, T&& x) noexcept {
    return named<default_v<static_string<S>>, T>{ std::forward<T>(x) };
  }

  template<fixed_storage S>
  inline constexpr auto operator""_from()
  {
    return []<class ...Args>(Args&&... args) {
      return into<default_v<static_string<S>>, Args...>{
        .args = std::forward_as_tuple(std::forward<Args>(args)...)
      };
    };
  }
}

// concept helpers
namespace mitama::mitamagic {
  template <static_string Tag, class _>
  struct is_named_as<Tag, named<Tag, _>> : std::true_type {};

  template <static_string Any, class _>
  struct is_named_any<named<Any, _>> : std::true_type {};
}

export namespace mitama {
  template <class ...> struct type_list {};

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
  template <named_any ...Named>
  constexpr auto sorted_indices = []<std::size_t ...Indices>(std::index_sequence<Indices...>) {
    using var_t = std::variant<std::integral_constant<std::size_t, Indices>...>;
    std::array arr{ var_t{std::in_place_index<Indices>}... };
    auto key = [](var_t v) {
      return std::visit([](auto i) { return list_element_t<decltype(i)::value, type_list<Named...>>::str; }, v);
    };
    auto cmp = [key](auto lhs, auto rhs) {
      return key(lhs) < key(rhs);
    };
    std::sort(arr.begin(), arr.end(), cmp);
    return arr;
  }(std::index_sequence_for<Named...>{});

  template <class, named_any...>
  struct sort;

  template <std::size_t ...I, named_any... Named>
  struct sort<std::index_sequence<I...>, Named...> {
    using type = type_list<list_element_t<sorted_indices<Named...>[I].index(), type_list<Named...>>...>;
  };

  export template <named_any ...Named>
  using sorted = sort<std::index_sequence_for<Named...>, Named...>::type;
}

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

// extensible record
export namespace mitama {
  template <class>
  class record;

  template <named_any ...Named>
    requires distinct<Named...> 
          && is_sorted<Named...>
  class record<type_list<Named...>>: protected Named...
  {
    template <std::size_t ...Indices, class ...Args>
    constexpr explicit record(std::index_sequence<Indices...>, std::tuple<Args...> args)
      noexcept((noexcept(Named{ std::get<Indices>(args) }) && ...))
      : Named{ std::get<Indices>(args) }...
    {}
  public:
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
  };

  template <named_any ...Named>
  record(Named...) -> record<sorted<Named...>>;
}
