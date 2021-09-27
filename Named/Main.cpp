///
/// Named parameters without any macros in C++20 
/// 

import named;
#include <iostream>
using namespace mitama::literals::named_literals;

void test(mitama::named<"a"_tag, int>) {}

int main() {
  test("a"_arg(1));
  mitama::record rec{ "a"_arg(1), "b"_arg(2) };

  std::cout << rec["a"_tag] << std::endl;
  std::cout << rec["b"_tag] << std::endl;
}
