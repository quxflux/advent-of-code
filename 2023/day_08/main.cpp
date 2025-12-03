#include <util.hpp>

#include <cassert>
#include <map>
#include <numeric>

namespace aoc {
namespace {
    using node_name = std::array<char, 3>;

    struct junction {
        node_name left {};
        node_name right {};
    };

    struct hasher {
        size_t operator()(const node_name& name) const
        {
            return std::hash<std::string_view> {}({ name.begin(), name.end() });
        }
    };

    auto read_input()
    {
        auto lines = load_input_by_line();

        const std::string pattern { lines.front() };

        constexpr auto parse = [](std::string_view line) -> std::pair<node_name, junction> {
            node_name node;
            node_name left;
            node_name right;

            std::ranges::copy(line.substr(0, 3), node.begin());
            std::ranges::copy(line.substr(7, 3), left.begin());
            std::ranges::copy(line.substr(12, 3), right.begin());

            return { node, { left, right } };
        };

        return std::pair { pattern, std::map<node_name, junction> { std::from_range, lines | std::views::drop(2) | std::views::transform(parse) } };
    }

    size_t
    calculate_num_steps_required(const auto& pattern, const auto& map, const node_name& from,
        const auto& reached_end)
    {
        node_name current_node = from;
        size_t num_steps = 0;

        for (auto dir : std::views::repeat(pattern) | std::views::join) {
            const auto [left, right] = map.at(current_node);
            current_node = dir == 'L' ? left : right;
            ++num_steps;

            if (reached_end(current_node))
                return num_steps;
        }

        std::unreachable();
    }

    size_t calculate_least_common_multiple(const std::span<const size_t> nums)
    {
        std::vector<size_t> current_values { std::from_range, nums };
        std::ranges::sort(current_values);

        return std::ranges::fold_left_first(current_values | std::views::reverse,
            [](const size_t lhs, const size_t rhs) {
                return std::lcm(lhs, rhs);
            })
            .value();
    }
}

size_t first_task()
{
    static constexpr node_name start_node { 'A', 'A', 'A' };
    static constexpr node_name end_node { 'Z', 'Z', 'Z' };

    const auto [pattern, map] = read_input();
    return calculate_num_steps_required(pattern, map, start_node, std::bind_front(std::equal_to {}, end_node));
}

size_t second_task()
{
    const auto [pattern, map] = read_input();

    constexpr auto is_start_node = [](const node_name& name) { return name[2] == 'A'; };
    constexpr auto is_end_node = [](const node_name& name) { return name[2] == 'Z'; };

    const std::vector<node_name> start_nodes { std::from_range,
        map | std::views::keys | std::views::filter(is_start_node) };

    const std::vector<size_t> cycle_lengths { std::from_range,
        start_nodes | std::views::transform([&](const node_name& node) {
            const auto num_steps = calculate_num_steps_required(pattern,
                map,
                node,
                is_end_node);
            assert(num_steps % pattern.size() == 0);
            return num_steps / pattern.size();
        }) };

    return calculate_least_common_multiple(cycle_lengths) * pattern.size();
}
}