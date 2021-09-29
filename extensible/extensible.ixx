module;
#include <algorithm>
#include <array>
#include <compare>
#include <iterator>
#include <string_view>
#include <ranges>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
export module extensible;

namespace mitama {
  template <std::default_initializable T>
  inline constexpr T default_v{};
}

namespace mitama {
  template<std::size_t N, class CharT>
  struct fixed_storage {
    static constexpr std::size_t size = N;
    using char_type = CharT;

    constexpr fixed_storage(CharT const (&s)[N])
      : fixed_storage(s, std::make_index_sequence<N>{}) {}
    template<std::size_t ...Indices>
    constexpr fixed_storage(CharT const (&s)[N], std::index_sequence<Indices...>)
      : s{ s[Indices]... } {}

    CharT const s[N];
  };
}

export namespace mitama {
  // non-type template enabled static string class
  // 
  // S: fixed_storage<N, CharT>
  template<auto S>
  struct static_string {
    static constexpr auto storage = S;
    using char_type = typename decltype(S)::char_type;

    static constexpr std::basic_string_view<char_type> const
    value = { storage.s, decltype(storage)::size };

    template <auto T>
      requires std::same_as<char_type, typename static_string<T>::char_type>
    constexpr std::strong_ordering operator<=>(static_string<T>) const noexcept {
      return static_string::value <=> static_string<T>::value;
    }
  };
}

export namespace mitama:: inline literals:: inline named_literals {
  // static string literal
  template<fixed_storage S>
  inline constexpr static_string<S> operator""_() {
    return {};
  }
}

// forward declarations of concept helper
namespace mitama::mitamagic {
  template <static_string, class>
  struct is_named_as : std::false_type {};

  template <class>
  struct is_named_any : std::false_type {};
}

// named concepts
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

    // basic constructor
    template <class ...Args>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : value( std::forward<Args>(args)... )
    {}

    // delegating constructor for emplace construction
    template <class ...Args>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(std::tuple<Args...> into)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage{ into, std::index_sequence_for<Args...>{} }
    {}

  private:
    // called between delegating constructor and basic constructor
    template <class ...Args, std::size_t ...Indices>
      requires std::constructible_from<T, Args...>
    constexpr explicit named_storage(std::tuple<Args...> into, std::index_sequence<Indices...>)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage{ std::get<Indices>(into)...}
    {}

  public:
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

    // basic constructor
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
  // Opaque-type that strict-typed via a phantom-parameter `Tag`.
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

    // basic constructor (for direct init)
    template <class U> requires std::constructible_from<T, U>
    constexpr explicit(!std::is_convertible_v<U, T>)
    named(U&& from)
      noexcept(std::is_nothrow_constructible_v<T, U>)
      : named_storage<T>{ std::forward<U>(from) }
    {}

    // emplace constructor
    template <class ...Args> requires std::constructible_from<T, Args...>
    constexpr named(into<Tag, Args...> into)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : named_storage<T>{ into.args }
    {}

    // accessors
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

  // Internal compiler error:
  // [ TODO: Minimize the problem and submit a bug report. ]
  //  
   template <static_string Tag>
   class named<Tag, void> {
   public:
     static constexpr std::string_view str = decltype(Tag)::value;
     static constexpr static_string tag = Tag;
   };


  // Overloading to make it easier to build `named`.
  // [ Example:
  //    ```cpp
  //    named<"id"_, int> id = "id"_ <= 42; 
  //    ```
  //    -- end example ]
  template <auto S, class T>
  constexpr auto operator<=(static_string<S>, T&& x) noexcept {
    return named<default_v<static_string<S>>, T>{ std::forward<T>(x) };
  }
}

export namespace mitama:: inline literals:: inline named_literals {
  // UDL for emplace construction
  //
  // [ Example:
  //    ```cpp
  //    named<"aaa"_, std::string> aaa = "aaa"_from(3, 'a');
  //    ```
  //    -- end example ]
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

// Type List
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

  template <class>
  struct list_size;

  template <class... _>
  struct list_size<type_list<_...>> {
    static constexpr auto value = sizeof...(_);
  };

  template <class T>
  inline constexpr auto list_size_v = list_size<T>::value;
}

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
    std::array arr{ boxed_index{std::in_place_index<Indices>}... };
    auto key = [](boxed_index boxed) {
      return std::visit([](auto i) {
        return list_element_t<decltype(i)::value, type_list<Named...>>::str;
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
  template <class>
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
    static constexpr auto size = sizeof...(Named);

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
  };

  template <named_any ...Named>
  record(Named...) -> record<sorted<Named...>>;

  template <named_any ...Named>
  using record_type = record<sorted<Named...>>;

  template <class Record, static_string ...Required>
  concept has = []{
    auto tags = Record::tags;
    return (std::binary_search(tags.cbegin(), tags.cend(), decltype(Required)::value) && ...);
  }();
}
