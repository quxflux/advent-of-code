#include <util.hpp>

#include <cmath>

namespace aoc {
namespace {
    std::vector<int32_t> extract_ints(const std::string_view s)
    {
        return s | std::views::split(' ') | std::views::transform(as_string_view) | std::views::transform(&to_int<>) | std::ranges::to<std::vector>();
    }

    std::vector<std::vector<int32_t>> get_reports()
    {
        return load_input_by_line() | std::views::transform(extract_ints) | std::ranges::to<std::vector>();
    }

    bool is_safe_impl(const std::span<const int32_t> report)
    {
        const auto diffs = report | std::views::adjacent_transform<2>(std::minus {});

        if (const auto [min, max] = std::ranges::minmax(diffs); std::signbit(min) != std::signbit(max))
            return false;

        const auto [min, max] = std::ranges::minmax(
            diffs | std::views::transform([](const auto t) { return std::abs(t); }));

        return std::abs(min) >= 1 && std::abs(max) <= 3;
    }

    bool is_safe_with_dampener(const std::span<const int32_t> report)
    {
        if (is_safe_impl(report))
            return true;

        for (size_t i = 0; i < report.size(); ++i) {
            auto t = report | std::ranges::to<std::vector>();
            t.erase(std::next(t.begin(), i));

            if (is_safe_impl(t))
                return true;
        }

        return false;
    }
}

size_t first_task()
{
    return std::ranges::distance(get_reports() | std::views::filter(is_safe_impl));
}

size_t second_task()
{
    return std::ranges::distance(get_reports() | std::views::filter(is_safe_with_dampener));
}
}
