module;

#include <concepts>
#include <variant>

export module Mitama.Result.def:success;

using namespace std;

namespace mitama {
  export template <destructible T>
  class success_t {
    T value;
    using value_type = remove_cvref_t<T>;
  public:
    template <class U>
    constexpr success_t(U&& value) noexcept : value{ std::forward<U>(value) } {}

    value_type&        get() &       { return value; }
    value_type const&  get() const&  { return value; }
    value_type&&       get() &&      { return std::move(value); }
    value_type const&& get() const&& { return std::move(value); }
  };

  export template <destructible T>
  class success_t<T&> {
    reference_wrapper<T> ref;
    using value_type = remove_cvref_t<T>;
  public:
    template <class U>
    constexpr success_t(U& value) noexcept : ref{ value } {}

    value_type&        get() &       { return ref.get(); }
    value_type const&  get() const&  { return ref.get(); }
    value_type&&       get() &&      { return std::move(ref.get()); }
    value_type const&& get() const&& { return std::move(ref.get()); }
  };

  export template <class T>
  success_t(T&&) -> success_t<T>;

  export inline constexpr auto success = []<class T = std::monostate>(T&& from = {}) {
    return success_t{ std::forward<T>(from) };
  };

} //! namespace mitama
