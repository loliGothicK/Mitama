///
/// Named parameters without any macros in C++20
///

import meta_sort;
import named;
#include <iostream>
using namespace mitama::literals::named_literals;

void date_impl(int month, int day)
{
  std::cout << "month: " << month << " day: " << day << '\n';
}

void date(mitama::named<"month"_tag, int> m, mitama::named<"day"_tag, int> d)
{
  date_impl(m.value(), d.value());
}

void date(mitama::named<"day"_tag, int> d, mitama::named<"month"_tag, int> m)
{
  date_impl(m.value(), d.value());
}

int main()
{
  date("month"_arg = 12, "day"_arg = 31);
  date("day"_arg = 31, "month"_arg = 12);
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