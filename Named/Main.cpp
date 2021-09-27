///
/// Named parameters without any macros in C++20
///

import meta_sort;
import named;
#include <iostream>
#include <format>
using namespace mitama::literals::named_literals;
using namespace std::literals;

int main() {
  auto person = mitama::record{
    "id"_arg = 1234,
    "name"_arg = "Mitama"s,
  };

  std::cout << std::format("{}: {}", person["id"_tag], person["name"_tag]) << std::endl;
}

// void test(mitama::named<"name"_tag, std::string> name)
// {
//   std::cout << "name:" << *name << '\n';
//   std::cout << "length:" << name->length() << '\n';
// }

// int main()
// {
//   using namespace std::literals;
//   test("name"_arg("test"s));
// }