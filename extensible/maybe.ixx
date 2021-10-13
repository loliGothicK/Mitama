export module Mitama.Data.Maybe;
export import Mitama.Data.Maybe.def;
import Mitama.Data.Result.def;

// customisation point
export namespace mitama {
  // maybe[result[T, E]] -> result[maybe[T], E]
  template <is_maybe Maybe> requires is_result<typename Maybe::value_type>
  inline constexpr auto transpose(Maybe&& may)
    -> result<
        maybe<typename Maybe::value_type::success_type>,
        typename Maybe::value_type::failure_type
    >
  {
    if (!!may) {
      auto res = may.unwrap();
      if (!!res) {
        return success(res.unwrap());
      }
      else {
        return failure(res.unwrap());
      }
    }
    else {
      return success(nothing);
    }
  }

  template <class T>
  void transpose(T&&)
  {
    static_assert(
      [] { return false; }(),
      "`maybe<T>::transpose` can be used if and only if `T` is a `result<_, _>`.");
  }
}
