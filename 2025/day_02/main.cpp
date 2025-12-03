#include <ranges>
#include <set>
#include <util.hpp>
#include <vector>

namespace aoc {
namespace {

    std::vector<std::pair<int64_t, int64_t>> load_data()
    {
        return load_input() //
            | std::views::split(',') //
            | std::views::transform(as_string_view) //
            | std::views::transform([](const std::string_view& s) {
                  const auto pos = s.find('-');
                  const auto min = s.substr(0, pos);
                  const auto max = s.substr(pos + 1);
                  return std::make_pair(to_int<int64_t>(min), to_int<int64_t>(max));
              })
            | std::ranges::to<std::vector>();
    }

    constexpr int64_t log_10(int64_t n)
    {
        return n == 0 ? 0 : 1 + log_10(n / 10);
    }

    constexpr int64_t pow_10(int64_t n)
    {
        return n == 0 ? 1 : 10 * pow_10(n - 1);
    }

    constexpr int64_t select_digits(int64_t x, int64_t start, int64_t len)
    {
        const auto t = x / pow_10(start);
        return t - (t / pow_10(len)) * pow_10(len);
    }
}

size_t first_task()
{
    size_t sum = 0;
    for (auto [range_start, range_end] : load_data()) {
        while (range_start <= range_end) {
            const auto num_digits = log_10(range_start);
            if (num_digits % 2 != 0) {
                const auto next_power_of_10 = pow_10(num_digits);
                if (next_power_of_10 > range_end)
                    break;
                range_start = next_power_of_10;
                continue;
            }

            const auto len = num_digits / 2;
            const auto upper_half = select_digits(range_start, len, len);
            const auto lower_half = select_digits(range_start, 0, len);

            if (upper_half == lower_half) {
                sum += range_start;
            }

            range_start++;
        }
    }
    return sum;
}

size_t second_task()
{
    // 222222 is a valid combination for 22 22 22 and 222 222, so we need to
    // filter the duplicates.
    std::set<int64_t> seen;

    for (auto [range_start, range_end] : load_data()) {
        while (range_start <= range_end) {
            const auto num_digits = log_10(range_start);

            for (int64_t width = 1; width < num_digits; width++) {
                if (num_digits % width != 0)
                    continue;

                bool all_match = true;

                int64_t first = 0;
                for (int64_t start = 0; start + width <= num_digits; start += width) {
                    if (start == 0)
                        first = select_digits(range_start, start, width);
                    else
                        all_match &= first == select_digits(range_start, start, width);
                }

                if (all_match)
                    seen.insert(range_start);
            }
            range_start++;
        }
    }

    return std::ranges::fold_left_first(seen, std::plus {}).value();
}
}
