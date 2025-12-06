#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace aoc {

template <typename F, typename Ret, typename... Args>
concept invocable_r = std::is_invocable_r_v<Ret, F, Args...>;

template <typename T>
concept printable = requires(std::ostream& os, const T t) {
    {
        os << t
    } -> std::same_as<std::ostream&>;
};

constexpr auto is_digit = [](const char c) { return c >= '0' && c <= '9'; };
constexpr auto as_string_view = [](
                                    const std::constructible_from<std::string_view> auto& str) { return std::string_view { str }; };

template <typename T = int32_t>
constexpr auto to_int(const std::string_view s)
{
    T r = 0;
    if (std::from_chars(s.data(), s.data() + s.size(), r).ec == std::errc::invalid_argument)
        throw std::invalid_argument("not a number");

    return r;
}

inline std::string read_file(const std::filesystem::path& p)
{
    std::ifstream ifs(p);
    if (!ifs.good())
        throw std::invalid_argument("Couldn't find input");
    std::ostringstream oss;
    std::ranges::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(),
        std::ostream_iterator<char>(oss));
    return oss.str();
}

struct load_input_params {
    std::string file_name = "input.txt";
    std::filesystem::path dir = std::filesystem::path(std::source_location::current().file_name()).parent_path();
};

inline std::string_view load_input(const load_input_params& params = {})
{
    static const auto input = read_file(params.dir / params.file_name);
    return std::string_view { input };
}

inline auto load_input_by_line(const load_input_params& params = {})
{
    return load_input(params) | std::views::split('\n') | std::views::transform(as_string_view) | std::views::filter(std::not_fn(&std::string_view::empty));
}

template <typename T>
struct simple_mdarray {
    simple_mdarray(const size_t rows, const size_t cols)
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

template <typename T>
auto col_view(const ptrdiff_t col_idx, const simple_mdarray<T>& data)
{
    return std::views::iota(size_t { 0 }, static_cast<size_t>(data.rows())) //
        | std::views::transform([&, col_idx](const size_t r) {
              return data[r, col_idx];
          });
}

template <typename T>
simple_mdarray<T> load_mdarray(invocable_r<T, char> auto char_convert_f, const load_input_params& params = {})
{
    const auto lines = load_input_by_line(params) | std::ranges::to<std::vector>();

    const auto n_rows = lines.size();
    if (n_rows == 0)
        throw std::invalid_argument("empty map");

    const auto n_cols = lines.front().size();
    if (n_cols == 0)
        throw std::invalid_argument("empty map");

    simple_mdarray<T> r(n_rows, n_cols);
    std::ranges::transform(lines | std::views::join, r.data().begin(), char_convert_f);
    return r;
}

template <printable T>
std::ostream& operator<<(std::ostream& os, const simple_mdarray<T>& m)
{
    for (size_t r = 0; r < m.rows(); ++r) {
        for (size_t c = 0; c < m.cols(); ++c)
            os << m[r, c];
        os << '\n';
    }

    return os;
}

size_t first_task();
size_t second_task();
}

int main()
{
    std::cout << "first task: " << aoc::first_task() << '\n';
    std::cout << "second task: " << aoc::second_task() << '\n';
}
