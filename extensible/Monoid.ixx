module;
#include <type_traits>
#include <utility>
export module Mitama.Base.Data.Monoid;
export import Mitama.Base.Data.SemiGroup;
import Mitama.Base.Concepts.HasXXX;

export namespace mitama {
  struct commutative_tag {};
}

export namespace mitama {
  template <class BinaryOp>
  class commutative_fn
    : protected BinaryOp
    , private data_tag_with<BinaryOp, associative_tag, commutative_tag>::type
  {
    BinaryOp op;
  public:
    using tag = data_tag_with<BinaryOp, associative_tag, commutative_tag>::type;

    template <class Fn>
    constexpr explicit commutative_fn(Fn&& fn)
      : op(std::forward<Fn>(fn))
    {}

    template <class A, class B>
    requires semigroup<BinaryOp, std::common_reference_t<A, B>>
      constexpr auto operator()(A&& a, B&& b) const {
      return std::invoke(op, std::forward<A>(a), std::forward<B>(b));
    }
  };

  template <class BinaryOp>
  commutative_fn(BinaryOp&&) -> commutative_fn<BinaryOp>;

  inline constexpr auto commutative = [](auto&& op) {
    return commutative_fn{ std::forward<decltype(op)>(op) };
  };
}

export namespace mitama {
  // https://ncatlab.org/nlab/show/monoid

  template <class Op, class T>
  concept monoid = semigroup<Op, T>
                && std::is_base_of_v<commutative_tag, Op>;
}
