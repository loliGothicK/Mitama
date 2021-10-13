module;

export module Mitama.Data.Result;
export import Mitama.Data.Result.def;
import Mitama.Data.Maybe.def;

// customisation point
export namespace mitama {
  // result[maybe[T], E] -> maybe[result[T, E]]
  template <is_result Result> requires is_maybe<typename Result::success_type>
  inline constexpr auto transpose(Result&& res)
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

}