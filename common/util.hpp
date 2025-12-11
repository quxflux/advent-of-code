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

inline std::string_view load_input(const std::source_location& loc = std::source_location::current())
{
    static const auto input = read_file(std::filesystem::path(loc.file_name()).parent_path() / "input.txt");
    return std::string_view { input };
}

inline auto load_input_by_line(const std::source_location& loc = std::source_location::current())
{
    return load_input(loc) | std::views::split('\n') | std::views::transform(as_string_view) | std::views::filter(std::not_fn(&std::string_view::empty));
}

template <typename T>
struct simple_mdarray {
    using value_type = T;

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
concept mdarray_like = std::same_as<simple_mdarray<typename T::value_type>, std::remove_cvref_t<T>>;

template <mdarray_like T>
auto col_view(const ptrdiff_t col_idx, T& data)
{
    return std::views::iota(size_t { 0 }, static_cast<size_t>(data.rows())) //
        | std::views::transform([&, col_idx](const size_t r) {
              return data[r, col_idx];
          });
}

template <mdarray_like T>
auto row_view(const ptrdiff_t row_idx, T& data)
{
    return data.data().subspan(row_idx * data.cols(), data.cols());
}

template <mdarray_like T>
auto colwise(const T& data)
{
    return std::views::iota(size_t { 0 }, static_cast<size_t>(data.cols())) //
        | std::views::transform([&](const size_t c) {
              return col_view(c, data);
          });
}

template <typename T>
simple_mdarray<T> load_mdarray(invocable_r<T, char> auto char_convert_f, const std::source_location& loc = std::source_location::current())
{
    const auto lines = load_input_by_line(loc) | std::ranges::to<std::vector>();

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

struct progress {
    float t = 0;
};

std::ostream& operator<<(std::ostream& os, const progress& p)
{
    constexpr auto width = 50;

    const size_t pos = static_cast<size_t>((width - 2) * p.t);
    os << '[';
    for (size_t i = 0; i < width - 2; ++i) {
        if (i < pos)
            os << '=';
        else if (i == pos)
            os << '>';
        else
            os << ' ';
    }
    os << "] " << static_cast<size_t>(p.t * 100.0f) << " %\r";
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
