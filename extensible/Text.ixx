module;
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <concepts>
export module Mitama.Base.Data.Text;
import Mitama.Base.Concepts.DataKind;
import Mitama.Base.Functional.Infix;

export namespace mitama {
  inline constexpr auto intersperse
    = infix([](char i, std::string_view input) -> std::string
  {
    auto cat = [&](char c) { return std::string({ c, i }); };
    auto output = input 
                | std::views::transform(cat)
                | std::views::join
                | std::views::drop(1);
    std::string result{};
    result.reserve(input.size() * 2 - 1);
    std::ranges::copy(output, std::back_inserter(result));
    return result;
  });

  inline constexpr auto intercalate
    = infix([](std::string_view i, range_of<std::string_view> auto inputs) -> std::string
  {
    std::string output = "";
    std::string_view delim = "";
    for (auto input : inputs) {
      output.append(std::exchange(delim, i))
            .append(input);
    }
    return output;
  });
}
