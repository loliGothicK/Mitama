module;
#include <coroutine>
#include <optional>
#include <memory>
#include <iostream>
export module Mitama.Async.OneShot;
import Mitama.Async.Generator;
import Mitama.Maybe;

namespace mitama::async::oneshot {
  template <class T>
  struct sync {
    maybe<T> msg{ nothing };
  };
}

export namespace mitama::async::oneshot {
  template <class T>
  class sender {
    std::shared_ptr<oneshot::sync<T>> sync;
  public:
    sender(std::shared_ptr<oneshot::sync<T>> sync)
      : sync(sync)
    {}

    sender(sender const&) = delete;
    sender& operator=(sender const&) = delete;
    sender(sender&&) = default;
    sender& operator=(sender&&) = default;

    void send(std::convertible_to<T> auto msg) & {
      msg = msg;
    }
  };
}

export namespace mitama::async::oneshot {
  template <class T>
  struct receiver {
    struct promise_type;
    using handle = std::coroutine_handle<promise_type>;
    struct promise_type {
      maybe<T> value_;
      static auto get_return_object_on_allocation_failure() { return receiver{ nullptr }; }
      auto get_return_object() { return receiver{ handle::from_promise(*this) }; }
      auto initial_suspend() { return std::suspend_always{}; }
      auto final_suspend() noexcept { return std::suspend_always{}; }
      void unhandled_exception() { std::terminate(); }
      void return_value(maybe<T> value) {
        value_ = value;
      }
      auto yield_value(maybe<T> value) {
        value_ = value;
        return std::suspend_always{};
      }
    };
    bool next() { return coro ? (coro.resume(), !coro.done()) : false; }
    maybe<T> recv() { return coro.promise().value_; }
    receiver(receiver const&) = delete;
    receiver(receiver&& rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
    ~receiver() { if (coro) coro.destroy(); }
  private:
    receiver(handle h) : coro(h) {}
    handle coro;
  };
}

namespace mitama::async::oneshot {
  template <class T>
  receiver<T> generate(std::shared_ptr<oneshot::sync<T>> sync) {
    if (auto value = sync->msg; value) co_return value;
    else co_yield value;
  }
}

export namespace mitama::async::oneshot {

  template <class T>
  auto channel() {
    auto sync = std::make_shared<oneshot::sync<T>>();
    return std::pair{ sender{sync}, generate(sync) };
  }
}