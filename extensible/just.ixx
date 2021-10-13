module;

#include <utility>

export module Mitama.Data.Maybe.def:just;

namespace mitama {

  // factory class
  template <class T>
  class just_t {
    T value;
    using value_type = std::remove_cvref_t<T>;
  public:
    template <class U>
    constexpr just_t(U&& value) noexcept : value{ std::forward<U>(value) } {}

    value_type&        get() &       { return value; }
    value_type const&  get() const&  { return value; }
    value_type&&       get() &&      { return std::move(value); }
    value_type const&& get() const&& { return std::move(value); }
  };

  template <class T>
  class just_t<T&> {
    std::reference_wrapper<T> ref;
    using value_type = std::remove_cvref_t<T>;
  public:
    template <class U>
    constexpr just_t(U& value) noexcept : ref{ value } {}

    value_type&        get() &       { return ref.get(); }
    value_type const&  get() const&  { return ref.get(); }
    value_type&&       get() &&      { return std::move(ref.get()); }
    value_type const&& get() const&& { return std::move(ref.get()); }
  };

  // factory method
  export inline constexpr auto just = []<class T>(T&& from) {
    return just_t{ std::forward<T>(from) };
  };

} //! namepsace mitama
