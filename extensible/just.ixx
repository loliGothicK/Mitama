module;

#include <utility>

export module Mitama.Maybe.def:just;

namespace mitama {

  // factory class
  template <class T> struct just_t {
    T value;
  };

  // factory method
  export inline constexpr auto just = []<class T>(T&& from) {
    return just_t{ std::forward<T>(from) };
  };

} //! namepsace mitama
