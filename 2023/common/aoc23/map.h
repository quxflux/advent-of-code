#pragma once

#include <util.hpp>

#include <cstdint>
#include <ostream>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace aoc {

template <typename T>
using map = simple_mdarray<T>;

template <typename T>
constexpr auto index_view(const map<T>& m)
{
    return std::views::cartesian_product(std::views::iota(size_t { 0 }, m.rows()),
        std::views::iota(size_t { 0 }, m.cols()));
}

template <typename T>
map<T> read_map(const std::string_view char_2d, invocable_r<T, char> auto char_convert_f)
{
    const auto lines = char_2d | std::views::split('\n') | std::views::transform(as_string_view) | std::ranges::to<std::vector>();

    const auto n_rows = lines.size();
    if (n_rows == 0)
        throw std::invalid_argument("empty map");

    const auto n_cols = lines.front().size();
    if (n_cols == 0)
        throw std::invalid_argument("empty map");

    map<T> r(n_rows, n_cols);
    std::ranges::transform(lines | std::views::join, r.data().begin(), char_convert_f);
    return r;
}
}