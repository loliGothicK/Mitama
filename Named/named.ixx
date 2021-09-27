module;
#include <utility>
#include <string_view>
#include <type_traits>
#include <compare>
#include <array>
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

/// <summary>
/// `named<"tag"_tag, T>` and `"tag"_arg`
/// </summary>
export namespace mitama {
  // `named`: opaque-type that strict-typed via phantom-type `Tag`.
  //! Class Types in Non-Type Template Parameters [P0732R2]
  template <static_string Tag, class T>
  struct named {
    static constexpr std::string_view tag = decltype(Tag)::value;
    T value;

    constexpr auto operator[](decltype(Tag)) const noexcept -> T {
      return value;
    }
  };
  
  // `arg_t`: placeholder-type that strict-typed via phantom-type `Tag`.
  //! Class Types in Non-Type Template Parameters [P0732R2]
  template <static_string Tag>
  struct arg_t {
    template <class T>
    constexpr auto operator()(T&& x) const noexcept {
      return named<Tag, T>{ .value = std::forward<T>(x) };
    }

    template <class T>
    constexpr auto operator=(T&& x) const noexcept {
      return named<Tag, T>{ .value = std::forward<T>(x) };
    }
  };

  // factory
  //! Class Types in Non-Type Template Parameters [P0732R2]
  template <static_string Tag>
  inline constexpr arg_t<Tag> arg{};

  // arg literal
  //! Class Types in Non-Type Template Parameters [P0732R2]
  namespace literals:: inline named_literals {
    template<fixed_storage S>
    inline constexpr auto operator""_arg() {
      return arg< static_string<S>{} >;
    }
  }
}

// concept helpers
namespace mitama::mitamagic {
  template <static_string, class>
  struct is_named_as : std::false_type {};
  template <static_string Tag, class _>
  struct is_named_as<Tag, named<Tag, _>> : std::true_type {};

  template <class>
  struct is_named_any : std::false_type {};
  template <static_string Any, class _>
  struct is_named_any<named<Any, _>> : std::true_type {};
}

// concepts
export namespace mitama {
  template <class T, static_string Tag>
  concept named_as = mitamagic::is_named_as<Tag, T>::value;

  template <class T>
  concept named_any = mitamagic::is_named_any<T>::value;
}

// extensible record
export namespace mitama {
  template <named_any ...Named>
  class record: protected Named...
  {
  public:
    constexpr record(Named... init) : Named{ init }... {}
    using Named::operator[]...;
  };

  template <named_any ...Named>
  record(Named...) -> record<Named...>;
}
