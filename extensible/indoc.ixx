module;
#include <string_view>
#include <ranges>
#include <vector>
#include <cctype>
#include <algorithm>
#include <numeric>
export module Mitama.Utility.Extensible.Indoc;

export namespace mitama {
  inline constexpr auto indoc(std::string_view input) -> std::string {
    size_t pos = 0;
    std::string_view text = input;
    std::vector<std::string_view> lines{};
    while ((pos = text.find('\n')) != std::string_view::npos) {
      lines.emplace_back(text.substr(0, pos));
      text = text.substr(pos + 1, text.size());
      pos = 0;
    }
    auto nonEmpty = lines | std::views::filter([](auto line) { return not line.empty(); });
    auto x = std::ranges::min(nonEmpty | std::views::transform([](auto line) {
      auto pos = std::ranges::find_if_not(line, [](auto c) { return c == ' '; });
      return std::ranges::distance(line.begin(), pos);
    }));
    auto removed = lines | std::views::transform([&](auto line) {
      if (not line.empty()) line.remove_prefix(x);
      return std::string(line) + '\n';
    });
    return std::accumulate(begin(removed) + 1, end(removed), std::string{});
  }
}
