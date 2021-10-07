module;

#include <concepts>
#include <type_traits>

export module Mitama.Functional.Transpose;
import Mitama.Maybe.def;
import Mitama.Result.def;

using namespace std;

namespace mitama {
  struct transpose_closure {
    // maybe[result[T, E]] -> result[maybe[T], E]
    template <is_maybe Maybe>
    requires is_result<typename Maybe::value_type>
      constexpr auto operator()(Maybe const& may) const
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
    // result[maybe[T], E] -> maybe[result[T, E]]
    template <is_result Result>
    requires is_maybe<typename Result::success_type>
      constexpr auto operator()(Result const& res) const
      -> maybe<
          result<
            typename Result::success_type::value_type,
            typename Result::failure_type
      >>
    {
      if (!!res) {
        auto may = res.unwrap();
        if (!!may) {
          return just(success(may.unwrap()));
        }
        else {
          return nothing;
        }
      }
      else {
        return just(failure(res.unwrap_err()));
      }
    }
  };

  export inline constexpr transpose_closure transpose = {};

} //! namespace mitama
