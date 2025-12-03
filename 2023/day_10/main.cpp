#include <aoc23/map.h>
#include <util.hpp>

#include <set>
#include <stack>
#include <utility>

namespace aoc {
namespace {
    enum class field {
        none,
        NS,
        WE,
        NE,
        NW,
        SW,
        SE,
        start
    };

    map<field> get_input()
    {
        constexpr auto map_element = [](const char c) {
            constexpr auto map_values = std::to_array<std::pair<char, field>>({ //
                { '|', field::NS },
                { '-', field::WE },
                { 'L', field::NE },
                { 'J', field::NW },
                { '7', field::SW },
                { 'F', field::SE },
                { '.', field::none },
                { 'S', field::start } });

            return std::ranges::find(map_values, c, &std::pair<char, field>::first)->second;
        };

        return read_map<field>(load_input(), map_element);
    }

    using position = std::pair<ptrdiff_t, ptrdiff_t>;

    constexpr std::vector<position> possible_neighbors(const field p)
    {
        switch (p) {
        case field::none:
            return {};
        case field::NS:
            return { { -1, 0 },
                { 1, 0 } };
        case field::WE:
            return { { 0, -1 },
                { 0, 1 } };
        case field::NE:
            return { { -1, 0 },
                { 0, 1 } };
        case field::NW:
            return { { -1, 0 },
                { 0, -1 } };
        case field::SW:
            return { { 1, 0 },
                { 0, -1 } };
        case field::SE:
            return { { 1, 0 },
                { 0, 1 } };
        case field::start:
            return { { 1, 0 },
                { -1, 0 },
                { 0, 1 },
                { 0, -1 } };
        }

        std::unreachable();
    }

    std::vector<position>
    find_loop_inner(const position current_pos, const position rel_origin, const map<field>& m,
        std::set<position>& visited)
    {
        std::vector<std::vector<position>> found_paths_to_start;

        const auto current_value = m[current_pos.first, current_pos.second];

        if (current_value == field::start)
            found_paths_to_start.push_back({ current_pos });

        for (auto delta_pos : possible_neighbors(current_value)) {

            if (delta_pos == rel_origin)
                continue;

            const auto offset_pos = position { current_pos.first + delta_pos.first,
                current_pos.second + delta_pos.second };

            if (offset_pos.first < 0 || offset_pos.first >= m.rows() || //
                offset_pos.second < 0 || offset_pos.second >= m.cols())
                continue;

            if (visited.contains(offset_pos))
                continue;

            visited.insert(offset_pos);

            auto path = find_loop_inner(offset_pos, { -delta_pos.first, -delta_pos.second }, m, visited);

            if (!path.empty()) {
                path.insert(path.begin(), current_pos);
                found_paths_to_start.push_back(std::move(path));
            }
        }

        if (found_paths_to_start.empty())
            return {};

        return std::ranges::max(found_paths_to_start, std::less {}, [](const auto& t) { return t.size(); });
    }

    auto find_loop(const map<field>& m)
    {
        auto indices = index_view(m);

        const auto [start_row, start_col] = *std::ranges::find_if(indices, [&](const auto indices) {
            const auto [row, col] = indices;
            return m[row, col] == field::start;
        });

        std::set<std::pair<ptrdiff_t, ptrdiff_t>> visited;
        return find_loop_inner({ start_row, start_col }, { 0, 0 }, m, visited);
    }

    map<field> scale_up(const map<field>& m)
    {
        map<field> up_scaled(m.rows() * 3, m.cols() * 3);
        std::ranges::fill(up_scaled.data(), field::none);

        for (const auto [r, c] : index_view(m)) {
            const auto t = m[r, c];

            if (t == field::none)
                continue;

            up_scaled[r * 3, c * 3] = field::start;
            for (const auto [d_r, d_c] : possible_neighbors(t))
                up_scaled[r * 3 + d_r, c * 3 + d_c] = field::start;
        }

        return up_scaled;
    }

    map<field> scale_down(const map<field>& m)
    {
        map<field> down_scaled(m.rows() / 3, m.cols() / 3);
        std::ranges::fill(down_scaled.data(), field::none);

        for (const auto [r, c] : index_view(down_scaled)) {
            const auto t = m[r * 3, c * 3];

            if (t == field::none)
                continue;

            down_scaled[r, c] = t;
        }

        return down_scaled;
    }

    void flood_fill(const position start_pos, map<field>& m, const field fill_value = field::WE)
    {
        std::stack<position> stack;
        stack.push(start_pos);

        while (!stack.empty()) {
            const auto pos = stack.top();
            stack.pop();

            if (m[pos.first, pos.second] != field::none)
                continue;

            m[pos.first, pos.second] = fill_value;

            for (const auto delta_pos : std::to_array<position>({ { -1, 0 },
                     { 1, 0 },
                     { 0, -1 },
                     { 0, 1 } })) {
                const auto offset_pos = position { pos.first + delta_pos.first, pos.second + delta_pos.second };

                if (offset_pos.first < 0 || offset_pos.first >= m.rows() || //
                    offset_pos.second < 0 || offset_pos.second >= m.cols())
                    continue;

                if (m[offset_pos.first, offset_pos.second] != field::none)
                    continue;

                stack.push(offset_pos);
            }
        }
    }
}

size_t first_task()
{
    const auto map = get_input();
    return (find_loop(map).size() - 2) / 2 + 1;
}

size_t second_task()
{
    const auto input_map = get_input();
    auto masked_map = input_map;
    const auto loop = find_loop(input_map);

    std::ranges::fill(masked_map.data(), field::none);
    for (const auto [r, c] : loop)
        masked_map[r, c] = input_map[r, c];

    masked_map[loop.front().first, loop.front().second] = field::SW;

    map up_scaled = scale_up(masked_map);

    flood_fill({ 0, 0 }, up_scaled);
    flood_fill({ up_scaled.rows() - 1, 0 }, up_scaled);
    flood_fill({ 0, up_scaled.cols() - 1 }, up_scaled);
    flood_fill({ up_scaled.rows() - 1, up_scaled.cols() - 1 }, up_scaled);

    const auto down_scaled = scale_down(up_scaled);
    return std::ranges::count(down_scaled.data(), field::none);
}
}