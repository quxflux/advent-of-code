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
namespace detail {
    template <typename F, typename Ret, typename... Args>
    concept invocable_r = std::is_invocable_r_v<Ret, F, Args...>;

    template <typename T>
    concept printable = requires(const T t) {
        {
            std::declval<std::ostream&>() << t
        } -> std::same_as<std::ostream&>;
    };
}

template <typename T>
struct map {
    map(const size_t rows, const size_t cols)
        : rows_(rows)
        , cols_(cols)
        , data_(rows * cols)
    {
    }

    [[nodiscard]] constexpr T& operator[](const size_t row, const size_t col)
    {
        return data_[row * cols_ + col];
    }

    [[nodiscard]] constexpr const T& operator[](const size_t row, const size_t col) const
    {
        return data_[row * cols_ + col];
    }

    [[nodiscard]] constexpr size_t rows() const
    {
        return rows_;
    }

    [[nodiscard]] constexpr size_t cols() const
    {
        return cols_;
    }

    [[nodiscard]] constexpr std::span<T> data() { return { data_ }; }

    [[nodiscard]] constexpr std::span<const T> data() const { return { data_ }; }

private:
    size_t rows_;
    size_t cols_;
    std::vector<T> data_;
};

template <detail::printable T>
std::ostream& operator<<(std::ostream& os, const map<T>& m)
{
    for (size_t r = 0; r < m.rows(); ++r) {
        for (size_t c = 0; c < m.cols(); ++c)
            os << m[r, c];
        os << '\n';
    }

    return os;
}

template <typename T>
constexpr auto index_view(const map<T>& m)
{
    return std::views::cartesian_product(std::views::iota(size_t { 0 }, m.rows()),
        std::views::iota(size_t { 0 }, m.cols()));
}

template <typename T>
map<T> read_map(const std::string_view char_2d, detail::invocable_r<T, char> auto char_convert_f)
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