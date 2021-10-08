///
/// # Mitama.Extensible
/// 
/// The library with new C++20 features for raw polymorphism.
///
#include <chrono>
#include <coroutine>
import Mitama.Data.Extensible.Record;
import Mitama.Functional.Extensible.Match;
import Mitama.Functional.Extensible.Lambda;
import Mitama.Inspector;
#include <iostream>
#include <format>
#include <map>
#include <string_view>
#include <vector>
#include <initializer_list>
#include <tuple>

using namespace mitama::literals;
using namespace std::literals;

int main() {
  using mitama::as;
  namespace inspect = mitama::inspect;
  {
    inspect::test<"Mitama.Data.Extensible.Record">()
      .inspect([](auto&& reporter) {
        // record type declaration
        using Person = mitama::record
                       < mitama::named<"name"_, std::string>
                       , mitama::named<"age"_, int>
                       >;
        Person john = Person{
            as<"name"_>("John"s),
            as<"age"_>(42),
        };
        Person tom = Person{
            as<"age"_>(42),
            as<"name"_>("Tom"s),
        };

        reporter << inspect::assert<"Mock">::mock_ok();
      });
  }
  {
    inspect::test<"Mitama.Functional.Extensible.Lambda">()
      .inspect([](auto&& reporter) {
        auto fn = "x"_ + 1;
        reporter << inspect::assert<"x+1">::equal(2, fn("x"_v = 1));
      });
  }
  {
    //  match |expr| with {
    //    match-arms...
    //  }
    // 
    // match-arm:
    //   
    //  - when(value) --> match-arm-body
    // 
    //  - when(placeholder)[guard-function] --> match-arm-body
    //                     ^~~~~~~~~~~~~~~~
    //                      (optional)
    //
    // match-arm-body:
    //  If the match expression `match |expr: T| with {...}` returns a value type of `R`,
    //  then `match-arm-body: Body` must satisfy one of the following conditions:
    // 
    //  - requires (kind<of<lambda>> body, has_rows<{placeholder}> record) { { body(record); } -> std::convertible_to<R>; }
    //  - requires (Body body, T arg) { { std::invoke(body, arg); } -> std::convertible_to<R>; }
    //  - requires (Body body) { { std::invoke(body); } -> std::convertible_to<R>; }
    //  - std::convertible_to<Body, R>
    // 
    inspect::test<"Mitama.Functional.Extensible.Match">()
      .inspect([](auto&& reporter) {
        using namespace mitama::match_expr::prelude;
        auto v = match | 1 + 2 | with{
          when("x"_)["x"_ % 2 == 0] --> 1,
          when("x"_)                --> [](auto x) { return 2; },
          when("x"_)                --> [] { return 3; },
          when("x"_)                --> ("x"_ * 4),
        };
        reporter << inspect::assert<"x+1">::equal(2, v);
      });
  }
}
