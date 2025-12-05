#include <util.hpp>

#include <optional>
#include <regex>
#include <utility>

namespace aoc {
namespace {
    size_t calc_num_matches(const simple_mdarray<char>& input, const ptrdiff_t row, const ptrdiff_t col)
    {
        const auto access = [&](const ptrdiff_t r, const ptrdiff_t c) {
            if (r < 0 || c < 0 || r >= input.rows() || c >= input.cols())
                return '.';

            return input[r, c];
        };

        size_t matches = 0;

        constexpr auto directions = std::to_array<std::pair<int, int>>({
            { -1, 0 }, // to top
            { 1, 0 }, // to bottom
            { 0, -1 }, // to left
            { 0, 1 }, // to right
            { -1, 1 }, // to top right
            { -1, -1 }, // to top left
            { 1, 1 }, // to bottom right
            { 1, -1 }, // to bottom left
        });

        for (const auto [row_mult, col_mult] : directions) {
            matches += //
                access(row + 0, col + 0) == 'X' && //
                access(row + 1 * row_mult, col + 1 * col_mult) == 'M' && //
                access(row + 2 * row_mult, col + 2 * col_mult) == 'A' && //
                access(row + 3 * row_mult, col + 3 * col_mult) == 'S';
        }

        return matches;
    }
}

size_t first_task()
{
    const auto input = load_mdarray<char>(std::identity {});

    size_t sum = 0;
    for (ptrdiff_t row = 0; row < input.rows(); ++row)
        for (ptrdiff_t col = 0; col < input.cols(); ++col)
            sum += calc_num_matches(input, row, col);

    return sum;
}

size_t second_task()
{
    const auto input = load_mdarray<char>(std::identity {});

    size_t sum = 0;
    for (ptrdiff_t row = 0; row < input.rows() - 2; ++row)
        for (ptrdiff_t col = 0; col < input.cols() - 2; ++col) {
            const std::array first = { input[row, col], input[row + 1, col + 1], input[row + 2, col + 2] };
            const std::array second = { input[row + 2, col], input[row + 1, col + 1], input[row, col + 2] };

            const bool first_matches = std::ranges::equal(first, std::array { 'M', 'A', 'S' }) || std::ranges::equal(first, std::array { 'S', 'A', 'M' });
            const bool second_matches = std::ranges::equal(second, std::array { 'M', 'A', 'S' }) || std::ranges::equal(second, std::array { 'S', 'A', 'M' });

            sum += first_matches && second_matches;
        }
    return sum;
}
}
