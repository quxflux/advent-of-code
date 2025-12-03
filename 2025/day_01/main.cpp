#include <ranges>
#include <util.hpp>
#include <vector>

namespace aoc {
namespace {

    std::vector<int> load_data()
    {
        return load_input_by_line()
            | std::views::transform([](const auto& s) {
                  const auto sign = s[0] == 'L' ? -1 : 1;
                  return sign * to_int(s.substr(1));
              })
            | std::ranges::to<std::vector>();
    }
}

size_t first_task()
{
    const auto data = load_data();

    int cnt = 0;
    int val = 50;
    for (const auto v : data) {
        val = (val + v) % 100;
        cnt += val == 0;
    }

    return cnt;
}

size_t second_task()
{
    const auto data = load_data();
    int cnt = 0;
    int val = 50;
    for (const auto v : data) {
        const auto add = v < 0 ? -1 : 1;

        // not very elegant, but works
        for (int i = 0; i < std::abs(v); ++i) {
            val += add;
            if (val == 100)
                val = 0;
            else if (val == -1)
                val = 99;
            cnt += val == 0;
        }
    }

    return cnt;
}
}
