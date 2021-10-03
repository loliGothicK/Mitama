
# Extensible Records in C++20

- Start Date: 2021-09-27

## Summary

Allows to use extensible records without macros in C++20.

## Motivation

I have wanted Extensible Records (a library in Haskell) for a long time.
The time has finally come. The language features we need to implement it are there in C++20!

## Row Polymorphism

Row polymorphism is a kind of polymorphism that allows one to write programs that are polymorphic on record field types (also known as rows, hence row polymorphism).

Here is a TypeScript example:

```typescript
type foo = { first: string, last: string };

const o = { first: "Foo", last: "Oof", age: 30 };  
const p = { first: "Bar", last: "Rab", age: 45 };  
const q = { first: "Baz", last: "Zab", gender: "m" };  

const main = <T extends foo>(o: T) => (p: T) => o.first + o.last

main(o) (p); // type checks  
main(o) (q); // type error
```

## Mitama.Data.Extensible.Record

In TypeScript, it is implemented as a language feature, but in C++20, there is no such feature, so we need to emulate it somehow.
The reference is the famous Haskell library **extensible** which emulates the same feature.

In the end I succeeded in making a library which allows the following syntax.

```cpp
import Mitama.Data.Extensible.Record;
#include <iostream>

using namespace mitama::literals;
using namespace std::literals;

int main() {
    // declare record type
    using Person = mitama::record
                   < mitama::named<"name"_, std::string>
                   , mitama::named<"age"_,  int>
                   >;

    // make record
    Person john = mitama::empty
                += "name"_ % "John"s
                += "age"_  % 42
                ;

    // access to rows
    john["name"_]; // "John"
    john["age"_];  // 42
}
```

## Guide-level explanation

### mitama::named

The syntax `"name"_` and `"age"_` is a UDL (User-Defined Literal).
These literals create a type `mitama::static_string` which is a structual type (non-type template enabled class).

Thus, `mitama::named<"age"_, int>` is a wrapped type of `int` named with `mitama::static_string`.

You can construct `mitama::named<_, T>` by applying `operator%(static_string, T)`.

```cpp
mitama::named age = "age"_ % 42;
```

`mitama::named` has some interfaces like `std::optional`.

```cpp
mitama::named age = "age"_ % 42;
age.value(); // 42

mitama::named name = "name"_ % "John"s;
name->length(); // 4
```

### mitama::record

`mitama::record<Rows... >` is constrained to only take `mitama::named` in `Rows...` and strings in `mitama::named` must be distinct.

When initialising a particular record type, the order of the initialisers is free, because the Row strings are distinct.

```cpp
using Person = mitama::record
               < mitama::named<"name"_, std::string>
               , mitama::named<"age"_,  int>
               >;
// OK
Person john = Person {
    "name"_ % "John"s,
    "age"_  % 42,
};
// Also OK
Person tom = Person {
    "age"_  % 42,
    "name"_ % "Tom"s,
};
```

We can make `mitama::record` with CTAD (Class template argument deduction).
In this case, `Rows...` of the record is inferred in the order of the initializers, so different record types are inferred depending on the order of the initializers.

```cpp
// john: record< named<"name"_, std::string>, named<"age"_, int> >
auto john = mitama::record {
    "name"_ % "John"s,
    "age"_  % 42,
};

// tom: record< named<"age"_, int>, named<"name"_, std::string> >
auto tom = mitama::record {
    "age"_  % 42,
    "name"_ % "Tom"s,
};
```

Use `mitama::shrink` to convert between records that have the same rows but in a different order.

```cpp
// decltype(john) _ = tom; // ERROR
decltype(john) _ = mitama::shrink(tom); // OK
```

In fact, `a = shrink(b);` converts `b: B` to `a: A` where `A ⊆ B`.

```cpp
using Person = mitama::record
               < mitama::named<"name"_, std::string>
               , mitama::named<"age"_,  int>
               >;

auto tom = mitama::record {
    "name"_   % "Tom"s,
    "age"_    % 42,
    "gender"_ % "m",   // an extra row
};

Person tom2 = mitama::shrink(tom); // OK
```

## Reference-level explanation

### How to specify string literals as a non-type template parameter

#### Basic idea

In order to specify string literals as a non-type template parameter, we first create a structural type class that holds `const CharT [N]`.
`CharT` is a structural, and an array of a structural type is also structural.
And the class such that all base classes and non-static data members are public and non-mutable and the types of all bases classes and non-static data members are structural types or (possibly multi-dimensional) array thereof is structural.
Thus, `fixed_string` below is a structural type.

```cpp
#include <string_view>
#include <type_traits>
#include <utility>

template<std::size_t N, class CharT>
struct fixed_string {
    static constexpr std::size_t size = N;
    using char_type = CharT;

    constexpr fixed_string(CharT const (&s)[N])
      : fixed_string(s, std::make_index_sequence<N>{}) {}
    template<std::size_t ...Indices>
    constexpr fixed_string(CharT const (&s)[N], std::index_sequence<Indices...>)
      : s{ s[Indices]... } {}

    CharT const s[N];
};
```

We can use `fixed_string` as a non-type template parameter, and CTAD will automatically infer `CharT` and `N` from string literals.

```cpp
template <fixed_string S>
struct static_string {
    using char_type = typename decltype(S)::char_type;
    static constexpr auto size = decltype(S)::size;
};

int main() {
    using ss1 = static_string<"test">;
    static_assert(std::same_as<char, typename ss1::char_type>);

    using ss2 = static_string<u"test">;
    static_assert(std::same_as<char16_t, typename ss2::char_type>);

    using ss3 = static_string<U"test">;
    static_assert(std::same_as<char32_t, typename ss3::char_type>);
}
```

#### Complete idea

In order to be able to handle `std::string_view` at compile time, `static_string` should have `static constexpr std::string_view`.

```cpp
template<fixed_string S>
struct static_string {
    using char_type = typename decltype(S)::char_type;
    static constexpr std::basic_string_view<char_type> const value = { S.s, decltype(S)::size };
};
```

Furthermore, creating a static_string with UDL gives an appearance like `"name"_`.
The `operator ""_()` allows us to pass string literals directly to the template parameter, so that we can construct a `static_string` using the `fixed_string` deduced by CATD.

```cpp
export namespace mitama:: inline literals:: inline static_string_literals{
  template <fixed_string S>
  inline constexpr auto operator ""_() noexcept {
    return static_string<S>{};
  }
}
```

### How to access to rows in a record

`mitama::named` has a protected member `operator[]` for record.

```cpp
template <static_string Tag, class T>
class named {
public:
    static constexpr std::string_view str = decltype(Tag)::value;
    // ...
protected:
    template <auto S> requires (static_string<S>::value == str)
    constexpr delctype(auto) operator[](static_string<S>) const noexcept -> T {
        return storage::deref();
    }
};
```

`Rows...` in `mitama::record<Rows... >` should all be `mitama::named`.
`mitama::record` inherits `Rows...` and make `operator[]` visible by using declaration.

```cpp
template <named_any ...Rows>
class record
    : protected Rows...
{
public:
    // ...
      
    using Rows::operator[]...;
};
```

This will enable to access rows through `operator[]` by ADL.

## Appendix A: development environment

Visual Studio 2022 17.0.0 Preview 4.1
