#include <util.hpp>

#include <unordered_map>

namespace aoc {
namespace {
    std::pair<int32_t, int32_t> extract_ints(const std::string_view s)
    {
        auto chunks = s | std::views::chunk_by([](const char a, const char b) {
            return (is_digit(a) && is_digit(b)) || (a == ' ' && b == ' ');
        });

        if (std::ranges::distance(chunks) != 3)
            throw std::invalid_argument("Invalid input");

        const auto first = std::string_view { std::ranges::begin(*chunks.begin()), std::ranges::end(*chunks.begin()) };
        const auto second = std::string_view { std::ranges::begin(*std::next(chunks.begin(), 2)),
            std::ranges::end(*std::next(chunks.begin(), 2)) };

        return { to_int(first), to_int(second) };
    }

    std::vector<std::pair<int32_t, int32_t>> get_numbers()
    {
        return load_input_by_line() | std::views::transform(extract_ints) | std::ranges::to<std::vector>();
    }
}

size_t first_task()
{
    const auto numbers = get_numbers();
    auto first = numbers | std::views::keys | std::ranges::to<std::vector>();
    std::ranges::sort(first);
    auto second = numbers | std::views::values | std::ranges::to<std::vector>();
    std::ranges::sort(second);

    size_t diffs = 0;
    for (const auto [a, b] : std::views::zip(first, second))
        diffs += std::abs(a - b);

    return diffs;
}

size_t second_task()
{
    const auto numbers = get_numbers();

    std::unordered_map<int32_t, int32_t> histogram;

    for (const auto t : numbers | std::views::values)
        histogram[t]++;

    size_t diff = 0;

    for (const auto t : numbers | std::views::keys)
        diff += t * histogram[t];

    return diff;
}
}
