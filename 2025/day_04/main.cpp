#include <util.hpp>

namespace aoc {
namespace {
    void calculate_density(const simple_mdarray<char>& input, simple_mdarray<int>& result)
    {
        for (ptrdiff_t row = 0; row < input.rows(); ++row)
            for (ptrdiff_t col = 0; col < input.cols(); ++col) {
                int sum = 0;

                for (ptrdiff_t row_offs = -1; row_offs <= 1; ++row_offs)
                    for (ptrdiff_t col_offs = -1; col_offs <= 1; ++col_offs) {
                        const auto final_row = row + row_offs;
                        const auto final_col = col + col_offs;

                        const auto inside_bounds = final_row >= 0 && final_row < input.rows() && final_col >= 0 && final_col < input.cols();
                        const auto is_self = row_offs == 0 && col_offs == 0;

                        if (!is_self)
                            sum += inside_bounds && input[final_row, final_col] == '@';
                    }

                result[row, col] = sum;
            }
    }
}

size_t first_task()
{
    const auto input = load_mdarray<char>(std::identity {});
    simple_mdarray<int> density(input.rows(), input.cols());
    calculate_density(input, density);

    return std::ranges::count_if(std::views::zip(input.data(), density.data()), [](const auto& input_and_density) {
        const auto [i, d] = input_and_density;
        return i == '@' && d < 4;
    });
}

size_t second_task()
{
    int sum = 0;

    auto input = load_mdarray<char>(std::identity {});
    simple_mdarray<int> density(input.rows(), input.cols());

    while(true)
    {
        const auto sum_old = sum;
        calculate_density(input, density);
        for (auto&& [i, d] : std::views::zip(input.data(), density.data()))
        {
            if (i == '@' && d < 4)
            {
                ++sum;
                i = 'x';
            }
        }

        if (sum_old == sum)
            break;
    }

    return sum;
}
}