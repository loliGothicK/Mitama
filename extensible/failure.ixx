module;

#include <concepts>

export module Mitama.Data.Result.def:failure;

using namespace std;

namespace mitama {
  export template <destructible T>
  class failure_t {
    T value;
    using value_type = remove_reference_t<T>;
  public:
    template <class U>
    constexpr failure_t(U&& value) noexcept : value{ std::forward<U>(value) } {}

    value_type&        get() &       { return value; }
    value_type const&  get() const&  { return value; }
    value_type&&       get() &&      { return std::move(value); }
    value_type const&& get() const&& { return std::move(value); }
  };

  export template <destructible T>
  class failure_t<T&> {
    reference_wrapper<T> ref;
    using value_type = remove_reference_t<T>;
  public:
    template <class U>
    constexpr failure_t(U& value) noexcept : ref{ value } {}

    value_type&        get() &       { return ref.get(); }
    value_type const&  get() const&  { return ref.get(); }
    value_type&&       get() &&      { return std::move(ref.get()); }
    value_type const&& get() const&& { return std::move(ref.get()); }
  };

  export template <class T>
  failure_t(T&&) -> failure_t<T>;

  export inline constexpr auto failure = []<class T>(T && from) {
    return failure_t{ std::forward<T>(from) };
  };

} //! namespace mitama
