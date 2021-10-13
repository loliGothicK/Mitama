module;
#include <type_traits>
#include <utility>
export module Mitama.Functional.Extensible;

export namespace mitama {
  template <class T> typename std::decay<T>::type decay_copy(T&& v)
  {
    return std::forward<T>(v);
  }
}
