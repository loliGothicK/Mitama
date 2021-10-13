module;
#include <concepts>
export module Mitama.Base.Concepts.HasXXX;

export namespace mitama {
  template <class T>
  concept has_type = requires { typename T::type; };

  template <class T, class As>
  concept has_type_as = has_type<T> && std::same_as<As, T>;

  template <class T, class ValueType = bool>
  concept has_value = requires { T::value; };

  template <class T, class ValueType = bool>
  concept has_value_as = requires { { T::value } -> std::same_as<ValueType>; };

  template <class T>
  concept has_tag = requires { typename T::tag; };
}
