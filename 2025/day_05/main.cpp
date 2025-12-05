#include <util.hpp>

namespace aoc {
namespace {
    using value_type = int64_t;

    std::tuple<std::vector<std::pair<value_type, value_type>>, std::vector<value_type>> load_data()
    {
        const auto contains_dash = [](const std::string_view s) { return s.find('-') != std::string::npos; };
        auto input = load_input_by_line();
        return std::tuple {
            input //
                | std::views::take_while(contains_dash) //
                | std::views::transform([](const std::string_view line) {
                      const auto dash = line.find('-');
                      return std::pair { to_int<value_type>(line.substr(0, dash)), to_int<value_type>(line.substr(dash + 1)) };
                  }) //
                | std::ranges::to<std::vector>(),
            input //
                | std::views::drop_while(contains_dash) //
                | std::views::transform(&to_int<value_type>) //
                | std::ranges::to<std::vector>()
        };
    }
}

size_t first_task()
{
    const auto [ranges, ingredients] = load_data();

    const auto is_fresh = [&](const value_type ingredient) {
        return std::ranges::any_of(ranges, [=](const auto& interval) {
            return interval.first <= ingredient && ingredient <= interval.second;
        });
    };

    return std::ranges::count_if(ingredients, is_fresh);
}

size_t second_task()
{
    const auto [ranges, _] = load_data();

    auto sorted_ranges = ranges;
    std::ranges::sort(sorted_ranges);

    auto current_interval = sorted_ranges.begin();
    for (auto it = sorted_ranges.begin() + 1; it != sorted_ranges.end(); ++it) {
        if (current_interval->second >= it->first) {
            current_interval->second = std::max(current_interval->second, it->second);
            it->first = -1; // mark as invalidated
        } else {
            current_interval = it;
        }
    }

    return std::ranges::fold_left_first(sorted_ranges | std::views::transform([](const auto& interval) -> value_type {
        if (interval.first == -1)
            return 0;
        return interval.second - interval.first + 1;
    }),
        std::plus {})
        .value();
}
}