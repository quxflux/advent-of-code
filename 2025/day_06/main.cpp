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

    auto load_data_task_1()
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

    constexpr value_type fold_with_op(auto&& range, const op op)
    {
        if (op == op::plus)
            return std::ranges::fold_left_first(range, std::plus {}).value();
        else
            return std::ranges::fold_left_first(range, std::multiplies {}).value();
    }
}

size_t first_task()
{
    const auto [data, ops] = load_data_task_1();

    value_type sum = 0;
    for (ptrdiff_t col = 0; col < data.cols(); ++col)
        sum += fold_with_op(col_view(col, data), ops[col]);

    return sum;
}

size_t second_task()
{
    constexpr auto is_whitespace = [](const char c) { return c == ' '; };
    constexpr auto drop_leading_whitespaces = std::views::drop_while(is_whitespace);

    const auto char_map = load_mdarray<char>(std::identity {});

    value_type sum = 0;
    std::vector<value_type> numbers;

    for (const auto& col : colwise(char_map) | std::views::reverse) {
        if (std::ranges::all_of(col, is_whitespace))
            continue;

        if (col.back() != std::to_underlying(op::plus) && col.back() != std::to_underlying(op::mult)) {
            numbers.push_back(to_int<value_type>(col | drop_leading_whitespaces | std::ranges::to<std::string>()));
            continue;
        }

        numbers.push_back(to_int<value_type>(col | std::views::take(char_map.rows() - 1) | drop_leading_whitespaces | std::ranges::to<std::string>()));
        sum += fold_with_op(numbers, static_cast<op>(col.back()));

        numbers.clear();
    }

    return sum;
}
}