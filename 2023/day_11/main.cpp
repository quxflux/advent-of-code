#include <aoc23/map.h>
#include <util.hpp>

#include <set>
#include <utility>

namespace aoc {
namespace {

    enum class field : char {
        empty = '.',
        galaxy = '#'
    };

    using position = std::pair<size_t, size_t>;

    std::ostream& operator<<(std::ostream& os, const field f)
    {
        return os << std::to_underlying(f);
    }

    map<field> read_input()
    {
        return read_map<field>(load_input(), [](const char c) { return static_cast<field>(c); });
    }

    auto row_view(auto& m, const size_t row_idx)
    {
        return m.data().subspan(m.cols() * row_idx, m.cols());
    }

    auto col_view(auto& m, const size_t col_idx)
    {
        return std::views::iota(size_t { 0 }, m.rows()) | std::views::transform([&m, col_idx](const auto& row) -> auto& { return m[row, col_idx]; });
    }

    auto calculate_galaxy_positions(const map<field>& m)
    {
        std::vector<position> galaxy_positions;
        std::ranges::copy(index_view(m) //
                | std::views::filter(
                    [&](const position& pos) { return m[pos.first, pos.second] == field::galaxy; }),
            std::back_inserter(galaxy_positions));

        return galaxy_positions;
    }

    auto num_duplicates(const auto& data)
    {
        return data.size() - (data | std::ranges::to<std::set>()).size();
    }

    auto gen_pairs(const std::span<const position> positions)
    {
        std::vector<std::pair<position, position>> all_pairs;
        all_pairs.reserve((positions.size() * (positions.size() - 1)) / 2);

        for (size_t i = 0; i < positions.size(); ++i)
            for (size_t k = i + 1; k < positions.size(); ++k)
                all_pairs.emplace_back(positions[i], positions[k]);

        return all_pairs;
    }

    auto calculate_expanded_rows_and_cols(const map<field>& m)
    {
        std::vector<size_t> to_expand_cols {};
        std::vector<size_t> to_expand_rows {};

        std::ranges::copy(std::views::iota(size_t { 0 }, m.cols()) | std::views::filter([&](const size_t col) {
            return std::ranges::count(col_view(m, col), field::empty) == m.rows();
        }),
            std::back_inserter(to_expand_cols));

        std::ranges::copy(std::views::iota(size_t { 0 }, m.rows()) | std::views::filter([&](const size_t row) {
            return std::ranges::count(row_view(m, row), field::empty) == m.cols();
        }),
            std::back_inserter(to_expand_rows));

        std::vector<size_t> combined_rows(m.rows() + to_expand_rows.size());
        std::ranges::copy(std::views::iota(size_t { 0 }, combined_rows.size()), combined_rows.begin());
        std::ranges::copy(to_expand_rows, combined_rows.begin() + m.rows());
        std::ranges::sort(combined_rows);

        std::vector<size_t> combined_cols(m.cols() + to_expand_cols.size());
        std::ranges::copy(std::views::iota(size_t { 0 }, combined_cols.size()), combined_cols.begin());
        std::ranges::copy(to_expand_cols, combined_cols.begin() + m.cols());
        std::ranges::sort(combined_cols);

        return std::pair { std::move(combined_rows), std::move(combined_cols) };
    }

    map<field> expand(const map<field>& m)
    {
        const auto [combined_rows, combined_cols] = calculate_expanded_rows_and_cols(m);
        map<field> expanded(combined_rows.size(), combined_cols.size());

        for (const auto [dst_row, src_row] : std::views::enumerate(combined_rows))
            for (const auto [dst_col, src_col] : std::views::enumerate(combined_cols))
                expanded[dst_row, dst_col] = m[src_row, src_col];

        return expanded;
    }
}

size_t first_task()
{
    const auto data = expand(read_input());

    const auto galaxy_positions = calculate_galaxy_positions(data);

    static constexpr auto dist = [](const position& a, const position& b) {
        return std::max(a.first, b.first) - std::min(a.first, b.first) + //
            std::max(a.second, b.second) - std::min(a.second, b.second);
    };

    return std::ranges::fold_left(gen_pairs(galaxy_positions) | std::views::transform([](const auto pair) { return std::apply(dist, pair); }),
        size_t { 0 }, std::plus {});
}

size_t second_task()
{
    const auto data = read_input();
    const auto [combined_rows, combined_cols] = calculate_expanded_rows_and_cols(data);

    const auto galaxy_positions = calculate_galaxy_positions(data);

    constexpr size_t expansion_factor = 1'000'000;

    const auto dist = [&](const position& a, const position& b) {
        const auto max_row = std::max(a.first, b.first);
        const auto min_row = std::min(a.first, b.first);

        const auto max_col = std::max(a.second, b.second);
        const auto min_col = std::min(a.second, b.second);

        const auto row_begin = std::ranges::find(combined_rows, min_row);
        const auto row_end = std::ranges::find(combined_rows, max_row);
        const auto num_to_expand_rows = num_duplicates(std::ranges::subrange { row_begin, row_end });
        const auto num_regular_rows = max_row - min_row - num_to_expand_rows;

        const auto col_begin = std::ranges::find(combined_cols, min_col);
        const auto col_end = std::ranges::find(combined_cols, max_col);
        const auto num_to_expand_cols = num_duplicates(std::ranges::subrange { col_begin, col_end });
        const auto num_regular_cols = max_col - min_col - num_to_expand_cols;

        return num_regular_cols + num_to_expand_cols * expansion_factor + num_regular_rows + num_to_expand_rows * expansion_factor;
    };

    return std::ranges::fold_left(gen_pairs(galaxy_positions) //
            | std::views::transform([&](const auto pair) { return std::apply(dist, pair); }),
        size_t { 0 }, std::plus {});
}
}
