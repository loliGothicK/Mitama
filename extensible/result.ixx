module;

export module Mitama.Result;
export import Mitama.Result.def;
export import Mitama.Functional.Result;

export namespace mitama::use::result::all {
  using namespace mitama::functional;
  using mitama::result;
  using mitama::success;
  using mitama::failure;
}
