module;
#include <concepts>
export module Mitama.Utility.Extensible;

export namespace mitama {
  template <std::default_initializable T>
  inline constexpr T default_v{};
}
