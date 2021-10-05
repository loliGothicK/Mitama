///
/// # Mitama.Extensible
/// 
/// The library with new C++20 features for raw polymorphism.
///

import Mitama.Data.Extensible.Record;
import Mitama.Functional.Extensible.Match;
import Mitama.Functional.Extensible.Lambda;
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
  {
    // record type declaration
    using Person = mitama::record
                   < mitama::named<"name"_, std::string>
                   , mitama::named<"age"_,  int>
                   >;
    {
      // OK
      Person john = Person{
          as<"name"_>("John"s),
          as<"age"_>(42),
      };
      // Also OK
      Person tom = Person{
          as<"age"_>(42),
          as<"name"_>("Tom"s),
      };
    }

    {
      auto tom = mitama::record{
          as<"age"_>(42),
          as<"name"_>("Tom"s),
      };
      Person tom2 = mitama::shrink(tom);
    }
    // make record
    Person john = mitama::empty
                += as<"name"_>("John"s)
                += as<"age"_>(42)
                ;
    
    // field select and structured binding
    auto const& [name, age] = mitama::select<"name"_, "age"_>(john);

    std::cout << name << ": " << age << "\n";
    
    std::apply(
      [](auto name, auto age) { std::cout << name << ": " << age << "\n"; },
      mitama::select<"name"_, "age"_>(john));
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
    using namespace mitama::match_expr::prelude;

    auto v = match |1+2| with {
      when("x"_)["x"_ % 2 == 0] --> 4,
      when("x"_) --> [] (auto x) { return 42; },
      when("x"_) --> [] { return 42; },
      when("x"_) --> ("x"_ * 42),
    };

    std::cout << v << std::endl;
  }

  { // Named: initialization
    { mitama::named<"a"_, int>( 42 ); }
    { mitama::named<"a"_, int>{ 42 }; }
    // named<"a"_, int> _ = 42;     // ERROR: cannot convert to `named<_, T>` from `T`
    // named<"a"_, int> _ = { 42 }; // ERROR: cannot use an explicit constructor in copy-list-initialization
    { mitama::named<"a"_, std::vector<int>> _{ 1, 2 }; }
    { mitama::named<"a"_, std::vector<int>> _{ { 1,2,3 }, std::allocator<int>{} }; }
    { mitama::named<"a"_, int> _ = as<"a"_>(42); }
  }

  {
    // deduction-guides
    auto john = mitama::record {
      as<"id"_>(1234),
      as<"name"_>("John"s),
    };
    
    // field refinement
    mitama::has_rows<"id"_, "name"_> auto person = john;

    // accessor
    std::cout << person["id"_] << ", " << person["name"_] << "\n";

    // field spreading
    using person_t = decltype(john);
    using new_person_t = person_t::spread<mitama::named<"phone_number"_, std::string>>;

    // filed shrinking
    using old_person_t = new_person_t::shrink<"phone_number"_>;
    static_assert(std::same_as<person_t, old_person_t>);
  }
}

static_assert(std::same_as<char,     typename decltype("char"_)::char_type>);
static_assert(std::same_as<char16_t, typename decltype(u"char16_t"_)::char_type>);
static_assert(std::same_as<char32_t, typename decltype(U"char32_t"_)::char_type>);
static_assert(std::same_as<wchar_t,  typename decltype(L"wchar_t"_)::char_type>);
