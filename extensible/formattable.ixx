module;

#include <type_traits>
#include <variant>
#include <format>
#include <ranges>

export module Mitama.Concepts.Formattable;
using namespace std;

namespace mitama {
  export template <class T>
  concept tuple_like = requires (T t) {
    std::tuple_size<remove_cvref_t<T>>::value;
    std::get<0>(t);
  };

  export template <class Dict>
  concept dictionary = requires {
    typename remove_cvref_t<Dict>::key_type;
    typename remove_cvref_t<Dict>::mapped_type;
    requires requires (Dict dict, typename remove_cvref_t<Dict>::key_type key) {
      { dict[key] } -> same_as<typename remove_cvref_t<Dict>::mapped_type>;
    };
  };
}

namespace mitama {

  template <class T>
  struct formattable_element : false_type {};
  template <class>
  struct formattable_range : false_type {};
  template <class>
  struct formattable_dictionary : false_type {};
  template <class>
  struct formattable_tuple : false_type {};

  template <class T>
  struct formattable
    : disjunction<
        formattable_element<T>,
        formattable_range<T>,
        formattable_dictionary<T>,
        formattable_tuple<T>
    >
  {};

  template <class T>
  requires requires { typename formatter<T>; }
  struct formattable_element<T> : true_type {};

  template <>
  struct formattable_element<monostate>
    : true_type
  {};

  template <class Range>
    requires ranges::range<Range>
  struct formattable_range<Range>
    : formattable<ranges::range_value_t<Range>>
  {};


  template <class, class>
  struct formattable_tuple_impl;

  template <class Tuple, size_t ...Indices>
  struct formattable_tuple_impl<Tuple, index_sequence<Indices...>>
    : conjunction<formattable<tuple_element_t<Indices, Tuple>...>>
  {};

  template <class Tuple>
    requires tuple_like<Tuple>
  struct formattable_tuple<Tuple>
    : formattable_tuple_impl<Tuple, make_index_sequence<tuple_size_v<Tuple>>>
  {};

  template <class Dict>
    requires dictionary<Dict>
  struct formattable_dictionary<Dict>
    : conjunction<
        formattable<typename remove_cvref_t<Dict>::key_type>,
        formattable<typename remove_cvref_t<Dict>::mapped_type>
    >
  {};
}

namespace mitama {
  export template <class T>
  concept formattable = formattable<T>::value;
}
