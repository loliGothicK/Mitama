
# Extensible Records in C++20

- Start Date: 2021-09-27

## Summary

Allows to use extensible records without macros in C++20.

## Motivation

TODO

## Guide-level explanation

### **`static_string`: Non-type template enabled constexpr string**

### **`named<{tag}, T>`: opaque-type of `T` via static_string `{tag}`**

#### Dereference and Indirection

`operator*` and `operator->` are available.

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

void test(mitama::named<"name"_, std::string> name) {
    std::cout << "name:" << *name << '\n'
    std::cout << "length:" << name->length() << '\n';
}

int main() {
    test("name"_v = "John"s);
}
```

#### Lazy emplace construction

`arg_t::operator(Args&&... args)()` can construct an object type of `T` (where `std::constructible_from<T, Args...>`)
in `named<_, T>` from the arguments pack type of `Args...` (note that `Args...` is forwarding reference)
with expression `T( std::forward<Args>(args)... )`.

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

void print(mitama::named<"name"_, std::string> name)
{
    std::cout << "name:" << *name << '\n';
    std::cout << "length:" << name->length() << '\n';
}

int main()
{
    print("name"_v(5, 'A'));
}
```

#### Default arguments

Right-most parameters can have default arguments with following syntax:

```cpp
import named;
#include <iostream>
using namespace mitama::literals;

void print(mitama::named<"name"_, std::string> name = "name"_v("anonymous"))
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

TODO

## Compiler Support

|            |                version                |
| :--------: | :-----------------------------------: |
|    MSVC    | Visual Studio 2022 Preview 4 or later |
|    GCC     |   gcc version 12.0.0 (experimental)   |
| LLVM Clang |                  No                   |

## Unresolved questions

- N/A
