
# RFC: Named parameters without any macros in C++20

- Feature Name: named_params_via_cpp20
- Start Date: 2021-09-27

## Summary

Allows to use named-parameters without macros in C++20.

## Motivation

Without this RFC, one can use named parameters, but one must be declare some macros first.
Thus one makes code less visible, and this RFC introduces non-type templated opaque-type instead of macros.

## Guide-level explanation

### Changes between RFC and Siv3D

First, here is an example from [Reputeless/NamedParameter](https://github.com/Reputeless/NamedParameter) on the use of `NamedParameters`.

```cpp
namespace Arg
{
    SIV_MAKE_NAMED_PARAMETER(month);
    SIV_MAKE_NAMED_PARAMETER(day);
}

void Date_impl(int month, int day)
{
    std::cout << "month: " << month << " day: " << day << '\n';
}

void Date(Arg::month_<int> m, Arg::day_<int> d)
{
    Date_impl(m.value(), d.value());
}

void Date(Arg::day_<int> d, Arg::month_<int> m)
{
    Date_impl(m.value(), d.value());
}

int main()
{
    Date(Arg::month = 12, Arg::day = 31);
    Date(Arg::day = 31, Arg::month = 12);
}
```

Second, this RFC proposes the following code:

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

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
```

This RFC introduces new templates `named` and `tag_t` instead of the macro
and thus there are three breaking changes listed bellow:

| braeking changes |     Siv3D     |          RFC           |
| :--------------: | :-----------: | :--------------------: |
| macro definition |    needed     |    no longer needed    |
| named-parameter  |  `tag_<Ty>`   | `named<"tag"_tag, Ty>` |
|  named-argument  | `tag = value` |  `"tag"_arg = value`   |

It is good that macros are no longer used,
but the type names are a bit verbose,
which is a drawback of this RFC's implementation.

### Dereference and Indirection

`operator*` and `operator->` are also available in this RFC.

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

void test(mitama::named<"name"_tag, std::string> name) {
    std::cout << "name:" << *name << '\n'
    std::cout << "length:" << name->length() << '\n';
}

int main() {
    test("name"_arg = "John"s);
}
```

### Lazy emplace construction

`arg_t::operator(Args&&... args)()` can construct an object type of `T` (where `std::constructible_from<T, Args...>`)
in `named<_, T>` from the arguments pack type of `Args...` (note that `Args...` is forwarding reference)
with expression `T( std::forward<Args>(args)... )`.

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

void print(mitama::named<"name"_tag, std::string> name)
{
    std::cout << "name:" << *name << '\n';
    std::cout << "length:" << name->length() << '\n';
}

int main()
{
    print("name"_arg(5, 'A'));
}
```

### Default arguments

Right-most parameters can have default arguments with following syntax:

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

void print(mitama::named<"name"_tag, std::string> name = "name"_arg("anonymous"))
{
    std::cout << "name:" << *name << '\n';
    std::cout << "length:" << name->length() << '\n';
}

int main()
{
    print();
}
```

## Reference-level explanation

**-- IN PROGRESS --**

## Unresolved questions

- N/A
