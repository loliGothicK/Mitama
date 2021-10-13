module;
#include <functional>
#include <type_traits>
#include <utility>
export module Mitama.Base.Data.Group;
export import Mitama.Base.Data.Monoid;

export namespace mitama {
  struct group_operation_tag {};
}

export namespace mitama {
  template <class BinaryOp>
  class group_fn
    : private data_tag_with<BinaryOp, associative_tag, commutative_tag, group_operation_tag>::type
  {
    BinaryOp op;
  public:
    using tag = data_tag_with<BinaryOp, associative_tag, commutative_tag, group_operation_tag>::type;

    template <class Op>
    constexpr explicit group_fn(Op&& op)
      : op(std::forward<Op>(op))
    {}

    template <class A, class B>
      requires magma<BinaryOp, std::common_reference_t<A, B>>
    constexpr auto operator()(A&& a, B&& b) const {
      return std::invoke(op, std::forward<A>(a), std::forward<B>(b));
    }
  };

  template <class Op>
  group_fn(Op&&) -> group_fn<Op>;

  inline constexpr auto group_op = [](auto&& op) {
    return group_fn{ std::forward<decltype(op)>(op) };
  };
}

export namespace mitama {
  // https://ncatlab.org/nlab/show/group

  template <class Op, class T>
  concept group = monoid<Op, T> && std::is_base_of_v<group_operation_tag, Op>;
}
