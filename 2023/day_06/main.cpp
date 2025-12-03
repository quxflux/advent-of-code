#include <util.hpp>

namespace aoc {
namespace {
    struct race {
        size_t time = 0;
        size_t distance = 0;

        static constexpr size_t velocity = 1;
    };

    constexpr auto races = std::to_array<race>({
        { .time = 48, .distance = 255 },
        { .time = 87, .distance = 1288 },
        { .time = 69, .distance = 1117 },
        { .time = 81, .distance = 1623 },
    });

    constexpr auto calculate_min_max_winning_time(const race& r)
    {
        const auto start_times = std::views::iota(size_t { 0 }, r.time);

        const auto beats_distance = [&](const size_t start_time) {
            const auto remaining_time = r.time - start_time;

            if (remaining_time == 0)
                return false;

            const auto v = start_time * r.velocity;
            return remaining_time * v > r.distance;
        };

        const auto min_wait_time = std::ranges::partition_point(start_times, std::not_fn(beats_distance));
        const auto max_wait_time = std::ranges::partition_point(start_times, beats_distance);

        return std::pair { *min_wait_time, *(max_wait_time - 1) };
    }

    constexpr auto get_num_winning_start_times(const race& r)
    {
        const auto [min, max] = calculate_min_max_winning_time(r);
        return max - min + 1;
    }
}

size_t first_task()
{
    return std::ranges::fold_left_first(races | std::views::transform(&get_num_winning_start_times),
        std::multiplies {})
        .value();
}

size_t second_task()
{
    return get_num_winning_start_times({ .time = 48876981, .distance = 255128811171623 });
}
}
