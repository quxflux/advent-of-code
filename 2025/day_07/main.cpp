#include <cassert>
#include <util.hpp>

namespace aoc {
namespace {
    using value_type = int64_t;

    enum class field {
        empty = '.',
        start = 'S',
        splitter = '^',
        visited = '#',
    };

    void print(const simple_mdarray<field>& map)
    {
        for (size_t r = 0; r < map.rows(); ++r) {
            for (size_t c = 0; c < map.cols(); ++c)
                std::cout << static_cast<char>(map[r, c]);
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    size_t trace_ray(simple_mdarray<field>& map, size_t row, const size_t col)
    {
        if (row >= map.rows() || col >= map.cols())
            return 0;

        auto current = map[row, col];

        while (current != field::splitter && current != field::visited) {
            ++row;
            if (row >= map.rows())
                return 0;
            current = map[row, col];
        }

        if (current == field::visited)
            return 0;

        map[row, col] = field::visited;

        return 1 + trace_ray(map, row, col - 1) + trace_ray(map, row, col + 1);
    }

    auto load_map()
    {
        auto map = load_mdarray<field>([](const char c) {
            for (const auto t : { field::empty, field::start, field::splitter }) {
                if (static_cast<char>(t) == c)
                    return t;
            }
            throw std::invalid_argument("invalid character in map");
        });

        const auto start_pos = [&] {
            const auto first_row = row_view(0, map) | std::views::enumerate;
            const auto it = std::ranges::find_if(first_row, [](const auto rc) {
                const auto [r, val] = rc;
                return val == field::start;
            });

            if (it == first_row.end())
                throw std::invalid_argument("no start position found");

            const auto [idx, _] = *it;
            return idx;
        }();

        return std::tuple { std::move(map), start_pos };
    }
}

size_t first_task()
{
    auto [map, start_pos] = load_map();
    return trace_ray(map, 0, start_pos);
}

size_t second_task()
{
    auto [map, start_pos] = load_map();
    simple_mdarray<value_type> scores(map.rows(), map.cols());

    scores[0, start_pos] = 1;
    for (size_t r = 1; r < map.rows(); ++r) {
        std::ranges::copy(row_view(r - 1, scores), row_view(r, scores).begin());

        const auto indices_of_splitters = row_view(r, map)
            | std::views::enumerate
            | std::views::filter([&](const auto rc) {
                  const auto [c, val] = rc;
                  return val == field::splitter;
              })
            | std::views::transform([](const auto rc) {
                  const auto [c, _] = rc;
                  return c;
              })
            | std::ranges::to<std::vector>();

        for (const auto col : indices_of_splitters) {
            scores[r, col - 1] += scores[r, col];
            scores[r, col + 1] += scores[r, col];
            scores[r, col] = 0;
        }
    }

    return std::ranges::fold_left_first(row_view(scores.rows() - 1, scores), std::plus {}).value();
}
}