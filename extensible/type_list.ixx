module;
#include <cstddef>
export module Mitama.Data.Extensible.TypeList;

// Type List
export namespace mitama {
  template <class ...> struct type_list {};

  template <std::size_t, class>
  struct list_element {};

  template <std::size_t I, class Head, class ...Tail> requires (I == 0)
    struct list_element<I, type_list<Head, Tail...>>
  {
    using type = Head;
  };

  template <std::size_t I, class Head, class ...Tail> requires (I != 0)
    struct list_element<I, type_list<Head, Tail...>>
  {
    using type = typename list_element<I - 1, type_list<Tail...>>::type;
  };

  template <std::size_t I, class T>
  using list_element_t = list_element<I, T>::type;

  template <class>
  struct list_size;

  template <class... _>
  struct list_size<type_list<_...>>
  {
    static constexpr auto value = sizeof...(_);
  };

  template <class T>
  inline constexpr auto list_size_v = list_size<T>::value;
}
