#pragma once

#include <array>
#include <stdexcept>
#include <functional>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <source_location>
#include <ranges>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <vector>

namespace aoc {

    constexpr auto is_digit = [](const char c) { return c >= '0' && c <= '9'; };
    constexpr auto as_string_view = [](
            const std::constructible_from<std::string_view> auto &str) { return std::string_view{str}; };

    template<typename T=int32_t>
    constexpr auto to_int(const std::string_view s) {
        T r = 0;
        if (std::from_chars(s.data(), s.data() + s.size(), r).ec == std::errc::invalid_argument)
            throw std::invalid_argument("not a number");

        return r;
    }

    inline std::string read_file(const std::filesystem::path &p) {
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

    inline std::string_view load_input(const load_input_params &params = {}) {
        static const auto input = read_file(params.dir / params.file_name);
        return std::string_view{input};
    }

    inline auto load_input_by_line(const load_input_params &params = {}) {
        return load_input(params) | std::views::split('\n') | std::views::transform(as_string_view) |
               std::views::filter(std::not_fn(&std::string_view::empty));
    }

    size_t first_task();
    size_t second_task();
}

int main() {
    std::cout << "first task: " << aoc::first_task() << '\n';
    std::cout << "second task: " << aoc::second_task() << '\n';
}
