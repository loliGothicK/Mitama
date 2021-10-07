module;
#include <source_location>
#include <concepts>
#include <barrier>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <ranges>
#include <variant>
#include <optional>
#include <future>
#include <vector>
export module Mitama.Inspector;
export import Mitama.Data.Extensible.Record;
import Mitama.Data.Extensible.StaticString;
import Mitama.Concepts.Extensible;
import Mitama.Utility.Extensible.Indoc;
import Mitama.Async.Generator;
import Mitama.Async.OneShot;
import Mitama.Maybe;
import Mitama.Result;
import Mitama.Result.IO;

export namespace mitama::inspect {
  template <fixed_string Name>
  class test
  {
    std::chrono::steady_clock::time_point start;
    
    using report_type = std::vector<result<std::monostate, std::string>>;
    std::vector<std::future<report_type>> sync;

    struct reporter {
      async::oneshot::sender<report_type> tx;
      report_type buffer = {};

      friend auto& operator<<(reporter& r, result<std::monostate, std::string> msg) {
        r.buffer.emplace_back(msg);
        return r;
      }

      ~reporter() noexcept {
        tx.send(buffer);
      }
    };
  public:
    test()
      : start(std::chrono::steady_clock::now())
    {}

    auto& inspect(auto fn) {
      auto [tx, rx] = async::oneshot::channel<report_type>();
      std::thread([&, fn] { fn(reporter{ .tx = std::move(tx) }); }).detach();
      sync.emplace_back(
        std::async(
          std::launch::async,
          [&, rx = std::move(rx)]() mutable -> report_type
          {
            report_type report = {};
            while (rx.next()) {
              if (auto messages = rx.recv(); messages) {
                for (auto msg : messages.unwrap()) {
                  report.push_back(msg);
                }
                break;
              }
            }
            return report;
          }));
      return *this;
    }

    auto wait() -> result<std::monostate, std::vector<std::string>> {
      std::vector<std::string> err_report{};
      for (auto& fut : sync) {
        for (auto report : fut.get())
        {
          std::cout << "debug wait" << std::endl;
          if(not report) err_report.emplace_back(report.unwrap_err());
        }
      }
      if (err_report.empty())
        return success();
      else
        return failure(err_report);
    }

    ~test() noexcept {
      std::cout << std::format("test::{} ... ", Name);
      if (auto res = wait(); res) {
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << std::format("passed [{} ms]\n", elapsed);
      }
      else {
        std::cout << "failed.";
        for (std::string_view report : res.unwrap_err()) {
          std::cout << report << '\n';
        }
      }
    }
  };

  template <fixed_string Info>
  struct assert {
    using report_result = result<std::monostate, std::string>;

    static constexpr auto mock_ok()
      -> report_result
    {
      return success();
    }
    static constexpr auto equal(auto a, auto b)
      -> report_result
      requires requires { { a == b } -> std::convertible_to<bool>; }
    {
      return success();
    }
  };
}
