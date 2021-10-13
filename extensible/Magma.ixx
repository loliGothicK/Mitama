module;
#include <concepts>
#include <type_traits>
export module Mitama.Base.Data.Magma;

export namespace mitama {
  // https://ncatlab.org/nlab/show/magma

  template <class Op, class T>
  concept magma = requires (Op op, T a, T b) {
    { std::invoke(op, a, b) } noexcept -> std::convertible_to<T>;
  };
}
