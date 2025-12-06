#include <cassert>
#include <util.hpp>

namespace aoc {
namespace {
    using value_type = int64_t;

    enum class op {
        plus = '+',
        mult = '*'
    };

    auto extract_row(auto&& row)
    {
        return row //
            | std::views::chunk_by([](const auto a, const auto b) { return !(a == ' ' || b == ' '); }) //
            | std::views::transform(as_string_view) //
            | std::views::filter([](const auto s) { return s != " "; });
    }

    auto load_data()
    {
        const auto num_rows = std::ranges::distance(load_input_by_line());
        const auto num_cols = std::ranges::distance(extract_row(*load_input_by_line().begin()));

        simple_mdarray<value_type> numbers(num_rows - 1, num_cols);

        for (auto&& [row_idx, row] : std::views::enumerate(load_input_by_line() | std::views::take(num_rows - 1))) {
            for (auto&& [col_idx, val] : std::views::enumerate(extract_row(row))) {
                numbers[row_idx, col_idx] = to_int<value_type>(val);
            }
        }

        auto ops = extract_row(*(load_input_by_line() | std::views::drop(num_rows - 1)).begin()) | std::views::transform([](const auto s) {
            assert(s.length() == 1);
            return static_cast<op>(s[0]);
        }) | std::ranges::to<std::vector>();

        return std::tuple { numbers, ops };
    }


}

size_t first_task()
{
    const auto [data, ops] = load_data();

    value_type sum = 0;
    for (ptrdiff_t col = 0; col < data.cols(); ++col) {
        if (ops[col] == op::plus)
            sum += std::ranges::fold_left_first(col_view(col, data), std::plus {}).value();
        else
            sum += std::ranges::fold_left_first(col_view(col, data), std::multiplies {}).value();
    }

    return sum;
}

size_t second_task()
{
    return 0;
}
}