///
/// # Mitama.Extensible
/// 
/// The library with new C++20 features for raw polymorphism.
///

import Mitama.Data.Extensible.Record;
import Mitama.Data.Extensible.ADT;
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
  {
    // record type declaration
    using Person = mitama::record_type
                   < mitama::named<"name"_, std::string>
                   , mitama::named<"age"_,  int>
                   >;

    // make record
    Person john = mitama::empty
                += "name"_ % "John"s
                += "age"_  % 42
                ;
    
    // field select and structured binding
    auto const& [name, age] = mitama::select<"name"_, "age"_>(john);

    std::cout << name << ": " << age << "\n";
    
    std::apply(
      [](auto name, auto age) { std::cout << name << ": " << age << "\n"; },
      mitama::select<"name"_, "age"_>(john));
  }
  
  { // tagged union:
    //
    //!```rust
    //! enum Test {
    //!   Zero(int),
    //!   One(char),
    //! }
    //!```
    using test = mitama::union_type
                 < mitama::named<"0"_, int>
                 , mitama::named<"1"_, char>
                 >;

    test x{ "0"_ % 0 };

    // match expression ???
    auto v = match<int>(x) >>= mitama::with {
      "0"_then --> [](auto x) { return x; },
      "1"_then --> [](auto x) { return x; },
    };

    std::cout << v << '\n';
  }

  { // Named: initialization
    { mitama::named<"a"_, int>( 42 ); }
    { mitama::named<"a"_, int>{ 42 }; }
    // named<"a"_, int> _ = 42;     // ERROR: cannot convert to `named<_, T>` from `T`
    // named<"a"_, int> _ = { 42 }; // ERROR: cannot use an explicit constructor in copy-list-initialization
    { mitama::named<"a"_, std::vector<int>> _{ 1, 2 }; }
    { mitama::named<"a"_, std::vector<int>> _{ { 1,2,3 }, std::allocator<int>{} }; }
    { mitama::named<"a"_, int> _ = "a"_ % 42; }
  }

  {
    // deduction-guides
    auto john = mitama::record {
      "id"_   % 1234,
      "name"_ % "John"s,
    };
    
    // field refinement
    mitama::has<"id"_, "name"_> auto person = john;

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
