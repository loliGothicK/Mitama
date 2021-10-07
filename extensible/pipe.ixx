module;

#include <functional>
#include <utility>
#include <concepts>

export module Mitama.Functional.Pipe;

using namespace std;

export namespace mitama {
  template <class T, class Fn>
    requires regular_invocable<Fn, T&&>
  auto operator>(T&& a, Fn&& fn) {
    return invoke(forward<Fn>(fn), forward<T>(a));
  }
} //! namespace mitama::mitamagic
