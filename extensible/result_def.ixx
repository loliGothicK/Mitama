module;

#include <variant>
#include <concepts>
#include <utility>
#include <source_location>

export module Mitama.Data.Result.def;
export import :success;
export import :failure;
import Mitama.Data.Panic;

using namespace std;

namespace mitama::customisation_point::for_result {
  template <class T>
  inline constexpr auto _transpose(T&& res) { return transpose(std::forward<T>(res)); }
}

namespace mitama {
  export template <destructible T = monostate, destructible E = monostate>
  class result {
    variant<success_t<T>, failure_t<E>> storage;
  public:
    using success_type = remove_cvref_t<T>;
    using failure_type = remove_cvref_t<E>;

    template <class U> requires constructible_from<T, U>
    constexpr result(success_t<U> ok)
      : storage{ in_place_type<success_t<T>>, ok.get() }
    {}

    template <class U> requires constructible_from<E, U>
    constexpr result(failure_t<U> err)
      : storage{ in_place_type<failure_t<E>>, err.get() }
    {}

    // operator bool (able to use contextually convertible to bool)
    constexpr explicit operator bool() const noexcept {
      return holds_alternative<success_t<T>>(storage);
    }

    constexpr success_type& unwrap(source_location info = source_location::current()) & {
      if (bool(*this)) {
        return get<success_t<T>>(storage).get();
      }
      else {
        throw runtime_panic{
          "unwrap with failure value. [in {}, line: {}]",
          info.function_name(),
          info.line()
        };
      }
    }
    constexpr success_type& unwrap(source_location info = source_location::current()) const& {
      if (bool(*this)) {
        return get<success_t<T>>(storage).get();
      }
      else {
        throw runtime_panic{
          "unwrap with failure value. [in {}, line: {}]",
          info.function_name(),
          info.line()
        };
      }
    }
    constexpr success_type& unwrap(source_location info = source_location::current()) && {
      if (bool(*this)) {
        return move(get<success_t<T>>(storage).get());
      }
      else {
        throw runtime_panic{
          "unwrap with failure value. [in {}, line: {}]",
          info.function_name(),
          info.line()
        };
      }
    }
    constexpr success_type& unwrap(source_location info = source_location::current()) const&& {
      if (bool(*this)) {
        return move(get<success_t<T>>(storage).get());
      }
      else {
        throw runtime_panic{
          "unwrap with failure value. [in {}, line: {}]",
          info.function_name(),
          info.line()
        };
      }
    }

    constexpr failure_type& unwrap_err()& {
      if (!bool(*this)) {
        return get<failure_t<E>>(storage).get();
      }
      else {
        throw runtime_panic{ "unwrap_err with success value." };
      }
    }
    constexpr const failure_type& unwrap_err() const& {
      if (!bool(*this)) {
        return get<failure_t<E>>(storage).get();
      }
      else {
        throw runtime_panic{ "unwrap_err with success value." };
      }
    }
    constexpr failure_type&& unwrap_err() && {
      if (!bool(*this)) {
        return move(get<failure_t<E>>(storage).get());
      }
      else {
        throw runtime_panic{ "unwrap_err with success value." };
      }
    }
    constexpr const failure_type&& unwrap_err() const&& {
      if (!bool(*this)) {
        return move(get<failure_t<E>>(storage).get());
      }
      else {
        throw runtime_panic{ "unwrap_err with success value." };
      }
    }

    constexpr auto transpose() const {
      return customisation_point::for_result::_transpose(*this);
    }
  };

  namespace mitamagic {
    template <class>
    struct is_result: std::false_type {};

    template <class T, class E>
    struct is_result<result<T, E>> : std::true_type {};
  }

  export template <class T>
  concept is_result = mitamagic::is_result<remove_cvref_t<T>>::value;

} //! namespace mitama
