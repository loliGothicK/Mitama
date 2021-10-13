module;

#include <type_traits>
#include <utility>
#include <concepts>
#include <variant>

export module Mitama.Data.Result.def:success;

export namespace mitama {
  template <std::destructible T>
  class success_t {
    T value;
    using value_type = std::remove_cvref_t<T>;
  public:
    template <class U>
    constexpr success_t(U&& value) noexcept : value{ std::forward<U>(value) } {}

    value_type&        get() &       { return value; }
    value_type const&  get() const&  { return value; }
    value_type&&       get() &&      { return std::move(value); }
    value_type const&& get() const&& { return std::move(value); }
  };

  template <std::destructible T>
  class success_t<T&> {
    std::reference_wrapper<T> ref;
    using value_type = std::remove_cvref_t<T>;
  public:
    template <class U>
    constexpr success_t(U& value) noexcept : ref{ value } {}

    value_type&        get() &       { return ref.get(); }
    value_type const&  get() const&  { return ref.get(); }
    value_type&&       get() &&      { return std::move(ref.get()); }
    value_type const&& get() const&& { return std::move(ref.get()); }
  };

  template <class T>
  success_t(T&&) -> success_t<T>;

  inline constexpr auto success = []<class T = std::monostate>(T&& from = std::monostate{}) {
    return success_t{ static_cast<T&&>(from) };
  };

} //! namespace mitama
