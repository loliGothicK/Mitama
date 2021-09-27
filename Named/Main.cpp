///
/// Named parameters without any macros in C++20 
/// 

import meta_sort;
import named;
#include <iostream>
#include <concepts>
#include <boost/type_index.hpp>
using namespace mitama::literals::named_literals;

using list = mitama::type_list<
  std::integral_constant<int, 2>,
  std::integral_constant<int, 4>,
  std::integral_constant<int, 1>>;

using expected = mitama::type_list<
  std::integral_constant<int, 1>,
  std::integral_constant<int, 2>,
  std::integral_constant<int, 4>>;

using sorted_list
  = mitama::sorted<list,
      [](auto a, auto b){
        return decltype(a)::type::value < decltype(b)::type::value;
      }>;

static_assert(std::same_as<expected, sorted_list>);



void test(mitama::named<"name"_tag, std::string> name) {
  std::cout << "name:" << *name << '\n';
  std::cout << "length:" << name->length() << '\n';
}

int main() {
  using namespace std::literals;

  test("name"_arg("test"s));

  // records
  mitama::record rec{
    "a"_arg = 1,
    "b"_arg = 2
  };

  std::cout << rec["a"_tag] << '\n';
  std::cout << rec["b"_tag] << '\n';
}
