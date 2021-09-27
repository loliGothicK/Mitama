module;
#include <utility>
#include <string_view>
#include <type_traits>
#include <compare>
#include <array>
#include <vector>
#include <ranges>
#include <algorithm>
#include <map>
export module named;

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
      static_string<S>::value <=> static_string<T>::value;
    }
  };

  // tag literal
  //! auto parameters
  namespace literals:: inline named_literals {
    template<fixed_storage S>
    inline constexpr static_string<S> operator""_tag() {
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
  concept named_as = mitamagic::is_named_as<Tag, T>::value;

  template <class T>
  concept named_any = mitamagic::is_named_any<T>::value;

  template <class ...Named>
  concept distinct = [] {
    if constexpr (sizeof...(Named) < 2) return true;
    else {
      std::array keys{ Named::tag... };
      std::sort(keys.begin(), keys.end());
      return std::ranges::adjacent_find(keys) == keys.end();
    }
  }();
}


/// <summary>
/// `named`, `tag_t` and so on
/// </summary>
export namespace mitama {
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

    template <class ...Args>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : value { std::forward<Args>(args)... }
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

  // `named`: opaque-type that strict-typed via phantom-type `Tag`.
  template <static_string Tag, class T = std::void_t<>>
  class named: named_storage<T> {
    using storage = named_storage<T>;
  public:
    static constexpr std::string_view tag = decltype(Tag)::value;

    constexpr named() = delete;

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
    template <named_any ...Named> requires distinct<Named...>
    friend class record;

    // for records
    constexpr auto operator[](decltype(Tag)) const noexcept -> T {
      return storage::deref();
    }
  };

  // `arg_t`: placeholder-type that strict-typed via phantom-type `Tag`.
  template <static_string Tag>
  struct arg_t {
    // Lazily constructs `named<Tag, T>` from `Args`
    // with a expression `named<Tag, T>{ std::forwad<Args>(args)... }`.
    template <class ...Args>
    constexpr auto operator()(Args&& ...args) const noexcept {
      return into<Tag, Args...>{
        .args = std::forward_as_tuple(std::forward<Args>(args)...)
      };
    }

    // Immediately constructs `named<Tag, T>`.
    template <class T>
    constexpr auto operator=(T&& x) const noexcept {
      return named<Tag, T>{ std::forward<T>(x) };
    }
  };

  // factory
  template <static_string Tag>
  inline constexpr arg_t<Tag> arg{};

  // arg literal
  namespace literals:: inline named_literals {
    template<fixed_storage S>
    inline constexpr auto operator""_arg() {
      return arg< static_string<S>{} >;
    }
  }
}

// concept helpers
namespace mitama::mitamagic {
  template <static_string Tag, class _>
  struct is_named_as<Tag, named<Tag, _>> : std::true_type {};

  template <static_string Any, class _>
  struct is_named_any<named<Any, _>> : std::true_type {};
}

// extensible record
export namespace mitama {
  template <named_any ...Named>
    requires distinct<Named...>
  class record: protected Named...
  {
  public:
    template <class ...Args>
    constexpr explicit record(Args&&... init)
      noexcept((noexcept(Named{ std::forward<Args>(init) }) && ...))
      : Named{ std::forward<Args>(init) }...
    {}
    
    using Named::operator[]...;
  };

  template <named_any ...Named>
  record(Named...) -> record<Named...>;
}
