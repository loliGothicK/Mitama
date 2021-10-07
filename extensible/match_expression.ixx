module;
#include <boost/hana/functional/overload_linearly.hpp>
#include <boost/hana/functional/fix.hpp>
#include <string_view>
#include <variant>
#include <stdexcept>
export module Mitama.Functional.Extensible.Match;
export import Mitama.Functional.Extensible.Lambda;
import Mitama.Data.Extensible.Record;
import Mitama.Concepts.Extensible;

export namespace mitama {
  template <class T>
  struct inspector {
    T actual;

    template <class... F>
    auto inspect(F&&... fn) const -> std::common_reference_t<std::invoke_result_t<F, T>...> {
      auto impl = boost::hana::fix([actual = actual](auto succ, auto head, auto... tail) {
        if constexpr (sizeof...(tail) == 0) {
          return head.check(std::as_const(actual))
            ? head(actual)
            : throw std::runtime_error{"non-exhausive case"};
        }
        else {
          return head.check(std::as_const(actual))
            ? head(actual)
            : succ(std::forward<decltype(tail)>(tail)...);
        }
      });

      return impl(std::forward<F>(fn)...);
    }
  };
  
  template <class T = void>
  struct match_ {};

  inline constexpr match_<> match{};

  template <class T>
  inline constexpr auto operator|(match_<>, T&& target) {
    return inspector<T>{
      .actual = std::forward<T>(target)
    };
  }

  template <class ...Fn>
  class [[nodiscard]] with {
    std::tuple<Fn...> fn;
  public:
    template <class... F>
    constexpr explicit with(F&&... fs) : fn{ std::forward<F>(fs)... } {}

    template <class Inspector>
    constexpr auto apply(Inspector&& inspector) const {
      return std::apply([&](auto&&... f) {
        return inspector.inspect(std::forward<decltype(f)>(f)...);
      }, fn);
    }
  };

  template <class ...Fn> with(Fn&&...) -> with<Fn...>;

  inline constexpr auto operator|(auto&& inspector, kind<of<with>> auto&& with) {
    return with.apply(inspector);
  }

  template <class Guard, class Body>
  struct match_arm {
    Guard _guard;
    Body _body;

    template <class T>
    constexpr auto check(T const& actual) const -> bool {
      return _guard.match(actual);
    }

    template <class T>
    constexpr decltype(auto) operator()(T&& actual) const {
      if constexpr (kind<Body, of<lambda>>) {
        auto args = _guard._when.make_record(actual);
        return _body(args);
      }
      else if constexpr (std::invocable<Body, T>) {
        return std::invoke(_body, std::forward<T>(actual));
      }
      else if constexpr (std::invocable<Body>) {
        (void)actual;
        return std::invoke(_body);
      }
      else {
        (void)actual;
        return _body;
      }
    }
  };

  template <class When, class Guard>
  struct guard_ {
    static constexpr auto is_match_gaurd = false;
    When _when;
    Guard _guard;

    template <class T>
    constexpr auto match(T&& actual) const -> bool {
      return _when.match(actual) && _guard(_when.make_record(actual));
    }

    constexpr auto operator--(int) && -> guard_ {
      return *this;
    }
  };

  template <class... Values>
  struct when_ {
    std::tuple<Values...> expected;

    template <class T>
    constexpr auto make_record(T&& actual) const {
      if constexpr (static_strings<std::tuple_element_t<0, std::tuple<Values...>>>) {
        return mitama::empty += as<std::tuple_element_t<0, std::tuple<Values...>>{}>(actual);
      }
      else {
        return mitama::empty;
      }
    }

    template <class T>
    constexpr auto match(T&& actual) const -> bool {
      return std::apply([&actual]<class... Ts>(Ts&&... expected) {
        if constexpr (static_strings<std::tuple_element_t<0, std::tuple<Values...>>>) {
          return true;
        }
        else {
          return ((expected == actual) || ...);
        }
      }, expected);
    }

    template <class Guard, class ...Values>
    inline constexpr auto operator[](Guard guard_fn) {
      return guard_<when_, Guard>{*this, guard_fn};
    }

    constexpr auto operator--(int) const {
      auto f = [](auto&&...) { return true; };
      return guard_<when_, decltype(f)>{*this, f};
    }
  };

  template <class ...Values>
  inline constexpr auto when(Values&&... values) {
    return when_<Values...>{ std::forward_as_tuple(std::forward<Values>(values)...) };
  }


  inline constexpr auto operator>(kind<of<guard_>> auto&& guard, auto&& body) {
    return match_arm{
      ._guard = std::forward<decltype(guard)>(guard),
      ._body  = std::forward<decltype(body)>(body)
    };
  }
}

export namespace mitama:: inline match_expr:: inline prelude {
  using mitama::match, mitama::with, mitama::when;
}
