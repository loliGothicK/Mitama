///
/// Named parameters without any macros in C++20
///

import named;

#include <iostream>
#include <format>
//#include <boost/type_index.hpp>
using namespace mitama::literals::named_literals;
using namespace std::literals;

int main() {
  { // operator""_from
    mitama::named<"a"_, std::string> _ = "a"_from(5, 'a');
  }

  { // Extensible Records
    auto person = mitama::record{
      "id"_ <= 1234,
      "name"_ <= "Mitama"s,
    };
    std::cout << std::format("{}: {}", person["id"_], person["name"_]) << std::endl;
  }
}

static_assert(std::same_as<char,     typename decltype("char"_)::char_type>);
static_assert(std::same_as<char16_t, typename decltype(u"char16_t"_)::char_type>);
static_assert(std::same_as<char32_t, typename decltype(U"char32_t"_)::char_type>);
static_assert(std::same_as<wchar_t,  typename decltype(L"wchar_t"_)::char_type>);
