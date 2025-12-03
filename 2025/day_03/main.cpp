#include <ranges>
#include <set>
#include <util.hpp>
#include <vector>

namespace aoc {
namespace {

    using bank = std::array<int, 100>;

    std::vector<bank> load_data()
    {
        return load_input_by_line() //
            | std::views::transform([](const std::string_view& s) {
                  bank r {};
                  for (size_t i = 0; i < s.size(); ++i)
                      r[i] = to_int(s.substr(i, 1));

                  return r;
              })
            | std::ranges::to<std::vector>();
    }

    size_t find_joltage_n(const bank b, const size_t n)
    {
        size_t result = 0;

        auto start = b.begin();
        for (size_t i = 0; i < n; ++i) {
            // limit lookahead
            const auto end = b.end() - (n - i) + 1;
            const std::forward_iterator auto max = std::ranges::max_element(start, end);
            start = max + 1;

            result = result * 10 + *max;
        }

        return result;
    }
}

size_t first_task()
{
    return std::ranges::fold_left_first(load_data() //
            | std::views::transform(std::bind_back(find_joltage_n, 2)),
        std::plus {})
        .value();
}

size_t second_task()
{
    return std::ranges::fold_left_first(load_data() //
            | std::views::transform(std::bind_back(find_joltage_n, 12)),
        std::plus {})
        .value();
}
}
