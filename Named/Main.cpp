///
/// Named parameters without any macros in C++20
///

import named;

#include <iostream>
#include <format>
using namespace mitama::literals::named_literals;
using namespace std::literals;

int main() {
  auto person = mitama::record{
    "id"_   <= 1234,
    "name"_ <= "Mitama"s,
  };
  std::cout << std::format("{}: {}", person["id"_], person["name"_]) << std::endl;
}
