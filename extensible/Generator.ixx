module;
#include <coroutine>
#include <utility>
export module Mitama.Async.Generator;

export namespace mitama::async {
  template <class T>
  struct generator {
    struct promise_type {
      T value_;

      auto get_return_object() {
        return generator{ *this };
      };

      auto initial_suspend() {
        return std::suspend_always{};
      }

      auto final_suspend() noexcept {
        return std::suspend_always{};
      }

      auto yield_value(T v) {
        value_ = v;
        return std::suspend_always{};
      }

      void return_void() {}

      void unhandled_exception() {
        throw;
      }
    };
    using coro_handle = std::coroutine_handle<promise_type>;

    struct iterator {
      coro_handle coro_;
      bool done_;

      iterator& operator++() {
        coro_.resume();
        done_ = coro_.done();
        return *this;
      }

      bool operator!=(const iterator& rhs) const {
        return done_ != rhs.done_;
      }

      T operator*() const {
        return coro_.promise().value_;
      }
    };

    ~generator() {
      if (coro_)
        coro_.destroy();
    }

    generator(generator const&) = delete;
    generator& operator=(generator const&) = delete;
    generator(generator&& rhs)
      : coro_(std::exchange(rhs.coro_, nullptr))
    {}
    generator& operator=(generator&& rhs) {
      coro_(std::exchange(rhs.coro_, nullptr));
      return *this;
    }

    iterator begin() {
      coro_.resume();
      return { coro_, coro_.done() };
    }

    iterator end() {
      return { {}, true };
    }

  private:
    explicit generator(promise_type& p)
      : coro_(coro_handle::from_promise(p)) {}

    coro_handle coro_;
  };
}
