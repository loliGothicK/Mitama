module;
#include <utility>
export module Mitama.Base.Data.SemiGroup;
export import Mitama.Base.Data.Magma;
import Mitama.Base.Concepts.DataKind;
import Mitama.Base.Concepts.HasXXX;

export namespace mitama {
  template <class... Tags>
  struct data_tag : private Tags... {};

  template <class Base, class... NewTag>
  struct data_tag_with {
    using type = data_tag<NewTag...>;
  };

  template <has_tag Base, class... Tags>
  struct data_tag_with<Base, Tags...>
    : data_tag_with<typename Base::tag, Tags...>
  {};

  template <class... Bases>
  struct data_tag_with<data_tag<Bases...>>
    : std::type_identity<data_tag<Bases...>>
  {};

  template <class... Bases, class Head, class... Tail>
  struct data_tag_with<data_tag<Bases...>, Head, Tail...>
    : std::conditional<
        (std::same_as<Bases, Head> || ...),
        typename data_tag_with<data_tag<Bases...>, Tail...>::type,
        typename data_tag_with<data_tag<Head, Bases...>, Tail...>::type
    >
  {};

  struct associative_tag {};
}

export namespace mitama {
  template <class BinaryOp>
  class associative_fn
    : private data_tag_with<BinaryOp, associative_tag>::type
  {
    BinaryOp op;
  public:
    using tag = data_tag_with<BinaryOp, associative_tag>::type;

    template <class Op>
    constexpr explicit associative_fn(Op&& op)
      : op(std::forward<Op>(op))
    {}

    template <class A, class B>
      requires magma<BinaryOp, std::common_reference_t<A, B>>
    constexpr auto operator()(A&& a, B&& b) const {
      return std::invoke(op, std::forward<A>(a), std::forward<B>(b));
    }
  };

  template <class Op>
  associative_fn(Op&&) -> associative_fn<Op>;

  inline constexpr auto associative = [](auto&& op) {
    return associative_fn{ std::forward<decltype(op)>(op) };
  };
}

export namespace mitama {
  // https://ncatlab.org/nlab/show/semigroup

  template <class Op, class T>
  concept semigroup = magma<Op, T> && std::is_base_of_v<associative_tag, Op>;
}
