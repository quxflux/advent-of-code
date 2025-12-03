#include <util.hpp>

#include <map>

namespace aoc {
namespace {
    const std::map<std::string_view, size_t> num_map {
        { "one", 1 },
        { "two", 2 },
        { "three", 3 },
        { "four", 4 },
        { "five", 5 },
        { "six", 6 },
        { "seven", 7 },
        { "eight", 8 },
        { "nine", 9 }
    };

    size_t extract_line_with_digits(const std::string_view line)
    {
        if (line.empty())
            return 0;

        const auto first = std::ranges::find_if(line, is_digit);
        const auto last = std::ranges::find_if(line | std::views::reverse, is_digit);

        return std::stoi(std::string { *first }) * 10 + std::stoi(std::string { *last });
    }

    size_t process_input(const auto& line_extract)
    {
        return std::ranges::fold_left(load_input_by_line() | std::views::transform(line_extract), size_t { 0 },
            std::plus {});
    }

    size_t extract_line_with_digits_and_words(const std::string_view line)
    {
        if (line.empty())
            return 0;

        const auto reversed_line = line | std::views::reverse;

        std::map<size_t, size_t> pos_and_values;

        const auto first_digit = std::ranges::find_if(line, is_digit);
        if (first_digit != line.end())
            pos_and_values[std::distance(line.begin(), first_digit)] = std::stoi(std::string { *first_digit });

        const auto last_digit = std::ranges::find_if(reversed_line, is_digit);
        if (last_digit != reversed_line.end())
            pos_and_values[line.size() - std::distance(reversed_line.begin(), last_digit)] = std::stoi(
                std::string { *last_digit });

        for (size_t i = 0; i < line.size(); ++i) {
            for (auto& [key, value] : num_map) {
                if (line.substr(i, key.size()) == key) {
                    pos_and_values[i] = value;
                    break;
                }
            }
        }

        return pos_and_values.begin()->second * 10 + pos_and_values.rbegin()->second;
    }
}

size_t first_task()
{
    return process_input(extract_line_with_digits);
}

size_t second_task()
{
    return process_input(extract_line_with_digits_and_words);
}
}
