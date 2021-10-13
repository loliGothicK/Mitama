module;
#include <coroutine>
#include <concepts>
#include <semaphore>
#include <latch>
#include <barrier>
#include <queue>
#include <memory>
#include <thread>
#include <chrono>
#include <optional>
#include <mutex>
export module Mitama.Async.Mpsc;
export import Mitama.Async.Generator;
import Mitama.Data.Maybe;

namespace mitama::async::mpsc {
  template <class T>
  class sync {
    std::counting_semaphore<>  token;
    std::counting_semaphore<1> guard;
    std::mutex mtx;
    std::queue<T> buffer;
  public:
    sync()
      : token(0)
      , guard(0)
      , mtx{}
      , buffer{}
    {}

    void produce(T msg)
    {
      std::lock_guard lock{ mtx };
      guard.acquire();
      buffer.push(msg);
      token.release();
      token.release();
    }

    auto try_consume() -> maybe<T>
    {
      if (!guard.try_acquire() || token.try_acquire())
        return nothing;

      auto ret = buffer.front();
      buffer.pop();
      guard.release();
      return just(ret);
    }
  };
}

export namespace mitama::async::mpsc {
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
      void return_void() {}
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

export namespace mitama::async::mpsc {
  template <class T>
  class sender {
    std::shared_ptr<mpsc::sync<T>> connect;
  public:
    sender(std::shared_ptr<mpsc::sync<T>> connect)
      : connect(connect)
    {}

    sender(sender const&) = default;
    sender& operator=(sender const&) = default;
    sender(sender&&) = default;
    sender& operator=(sender&&) = default;

    void send(std::convertible_to<T> auto msg)& {
      connect->produce(msg);
    }
  };
}

namespace mitama::async::mpsc {
  template <class T>
  receiver<T> make_receiver(std::shared_ptr<mpsc::sync<T>> connect) {
    while (connect.use_count() > 1) co_yield connect->try_consume();
  }
}

export namespace mitama::async::mpsc {
  template <class T>
  auto channel() {
    auto sync = std::make_shared<mpsc::sync<T>>();
    return std::pair{ sender(sync), make_receiver(sync) };
  }
}

