module;
#include <functional>
export module Mitama.Functional.Extensible.Lambda;
export import Mitama.Data.Extensible.Record;
export import Mitama.Concepts.Extensible;

#define MITAMA_LAMBDA_BINARY_FUNCTION(FUNCTOR, OPERATOR)                    \
  template <class A, class B>                                               \
    requires (static_strings<A> || kind<A, of<lambda>>                      \
          || static_strings<B> || kind<B, of<lambda>>)                      \
          && (!requires { std::decay_t<A>::is_match_gaurd; })               \
  inline constexpr auto operator OPERATOR(A && a, B && b) noexcept {        \
    return lambda<FUNCTOR, A, B>{ std::forward<A>(a), std::forward<B>(b) }; \
  }

#define MITAMA_LAMBDA_UNARY_FUNCTION(FUNCTOR, OPERATOR)                     \
  template <class A>                                                        \
    requires static_strings<A> || kind<A, of<lambda>>                       \
  inline constexpr auto operator OPERATOR(A && a) noexcept {                \
    return lambda<FUNCTOR, A>{ std::forward<A>(a) };                        \
  }

export namespace mitama {
  template <class Op, class... T>
  class lambda {
    std::tuple<T...> binds;

    template <auto S>
    constexpr decltype(auto) arg(static_string<S> tag, auto& record) const {
      return record[tag];
    }
    constexpr decltype(auto) arg(kind<of<lambda>> auto& fn, auto& record) const {
      return fn(record);
    }
    constexpr decltype(auto) arg(auto& a, auto& record) const {
      return a;
    }
  public:
    constexpr lambda(T... args) noexcept 
      : binds{ args... }
    {}

    constexpr auto operator()(named_any auto&&... named) const -> decltype(auto) {
      auto args = record{ named... };
      return std::apply([&]<class... Args>(Args&&... binds) {
        return std::invoke(Op{}, arg(std::forward<Args>(binds), args)...);
      }, binds);
    }

    constexpr auto operator()(kind<of<record>> auto&& args) const -> decltype(auto) {
      return std::apply([&]<class... Args>(Args&&... binds) {
        return std::invoke(Op{}, arg(std::forward<Args>(binds), args)...);
      }, binds);
    }
  };
}


export namespace mitama {
  // arithmetic operators
  MITAMA_LAMBDA_BINARY_FUNCTION(std::plus<>, +)
  MITAMA_LAMBDA_BINARY_FUNCTION(std::minus<>, -)
  MITAMA_LAMBDA_BINARY_FUNCTION(std::multiplies<>, *)
  MITAMA_LAMBDA_BINARY_FUNCTION(std::divides<>, /)
  MITAMA_LAMBDA_BINARY_FUNCTION(std::modulus<>, %)
  MITAMA_LAMBDA_UNARY_FUNCTION(std::negate<>, -)

  // relational operators
  MITAMA_LAMBDA_BINARY_FUNCTION(std::equal_to<>, == )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::not_equal_to<>, != )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::greater<>, > )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::less<>, < )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::greater_equal<>, >= )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::less_equal<>, <= )

  // bitwise operators
  MITAMA_LAMBDA_BINARY_FUNCTION(std::bit_and<>, & )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::bit_or<>, | )
  MITAMA_LAMBDA_BINARY_FUNCTION(std::bit_xor<>, ^ )
  MITAMA_LAMBDA_UNARY_FUNCTION(std::bit_not<>, ~)
}
