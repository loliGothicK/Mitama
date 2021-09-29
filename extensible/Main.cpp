///
/// Extensible Records in C++20
///

import Mitama.Extensible;
#include <iostream>
#include <format>
using namespace mitama::literals;
using namespace std::literals;

int main() {
  { // operator""_from
    mitama::named<"a"_, std::string> _ = "a"_from(5, 'a');
  }

  { // Extensible Records
    auto john = mitama::record{
      "id"_ << 1234,
      "name"_ << "John"s,
    };

    auto fn1 = [](mitama::has<"id"_, "name"_> auto person) {
      std::cout << std::format("{}: {}", person["id"_], person["name"_]) << std::endl;
    };

    fn1(john);

    using person_t = decltype(john);
    using new_person_t = person_t::spread<mitama::named<"phone_number"_, std::string>>;

    new_person_t tom {
      "id"_   << 1234,
      "name"_ << "Tom"s,
      "phone_number"_ << "123-4567-8910"s,
    };

    auto fn2 = [](mitama::has<"phone_number"_> auto person) {
      std::cout << std::format("phone_number: {}", person["phone_number"_]) << std::endl;
    };

    fn2(tom);

    using old_person_t = new_person_t::shrink<"phone_number"_>;
    static_assert(std::same_as<person_t, old_person_t>);

    [](mitama::records auto&&){}(tom);
  }
  {
    using Person = mitama::record_type
                   < mitama::named<"id"_,   int>
                   , mitama::named<"name"_, std::string>
                   >;

    Person _ = mitama::empty
             | "id"_   << 1234
             | "name"_ << "John"s
             ;
  }
}

static_assert(std::same_as<char,     typename decltype("char"_)::char_type>);
static_assert(std::same_as<char16_t, typename decltype(u"char16_t"_)::char_type>);
static_assert(std::same_as<char32_t, typename decltype(U"char32_t"_)::char_type>);
static_assert(std::same_as<wchar_t,  typename decltype(L"wchar_t"_)::char_type>);
