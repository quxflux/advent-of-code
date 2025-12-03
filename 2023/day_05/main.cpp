#include <util.hpp>

#include <map>
#include <regex>
#include <spanstream>

namespace aoc {
namespace {
    struct almanac {
        struct mapping {
            std::string from;
            std::string to;

            struct range {
                size_t start {};
                size_t len {};

                constexpr bool operator<(const range& rhs) const noexcept
                {
                    return start < rhs.start;
                }
            };

            std::map<range, size_t> source_to_dest;
        };

        std::vector<size_t> seeds;
        std::vector<mapping> mappings;
    };

    size_t map_source_to_dest(const almanac::mapping& mapping, const size_t value)
    {
        auto it = mapping.source_to_dest.upper_bound(almanac::mapping::range { value, 0 });

        if (it == mapping.source_to_dest.begin())
            return value;

        std::advance(it, -1);

        const auto d0 = value - it->first.start;
        const auto d1 = (it->first.start + it->first.len) - value;
        if (value >= it->first.start && value < it->first.start + it->first.len)
            return it->second + value - it->first.start;

        return value;
    }

    size_t resolve_seed(const std::span<const almanac::mapping> mappings, const size_t value)
    {
        size_t result = value;

        for (const auto& mapping : mappings)
            result = map_source_to_dest(mapping, result);

        return result;
    }

    almanac read_almanac()
    {
        const std::vector lines { std::from_range, load_input_by_line() };

        almanac result;

        result.seeds = { std::from_range,
            lines.front()
                | std::views::chunk_by([](const auto lhs, const auto rhs) {
                      return is_digit(lhs) == is_digit(rhs);
                  }) //
                | std::views::filter([](const auto& t) { return is_digit(t.front()); }) //
                | std::views::transform(
                    [](const auto& t) { return std::stoul(std::string { t.begin(), t.end() }); }) };

        const std::vector mapping_descriptors {
            std::from_range,
            lines
                | std::views::drop(2)
                | std::views::chunk_by([](const auto lhs, const auto rhs) { return !lhs.empty(); })
                | std::views::transform([](const auto t) {
                      return std::vector { std::from_range, t | std::views::take(std::ranges::size(t) - 1) };
                  })
        };

        result.mappings = { std::from_range,
            mapping_descriptors
                | std::views::transform([](const auto& lines) -> almanac::mapping {
                      almanac::mapping result;

                      {
                          const std::regex game_regex { R"(([a-z]+)-to-([a-z]+) map:)" };
                          const std::string first_line { lines.front() };
                          const std::sregex_iterator it { first_line.begin(), first_line.end(), game_regex };

                          result.from = it->str(1);
                          result.to = it->str(2);
                      }

                      for (const auto& line : lines | std::views::drop(1)) {

                          almanac::mapping::range source_range;
                          size_t destination_start;

                          std::ispanstream { line } >> destination_start >> source_range.start
                              >> source_range.len;
                          result.source_to_dest.insert({ source_range, destination_start });
                      }

                      return result;
                  }) };

        return result;
    }

    size_t solve_for_seeds(auto&& seeds, const std::span<const almanac::mapping> mappings)
    {
        constexpr auto min_f = [](const auto lhs, const auto rhs) { return std::min(lhs, rhs); };
        return std::ranges::fold_left_first(
            seeds | std::views::transform([=](const auto seed) { return resolve_seed(mappings, seed); }),
            min_f)
            .value();
    }
}

size_t first_task()
{
    const auto almanac = read_almanac();
    return solve_for_seeds(almanac.seeds, almanac.mappings);
}

size_t second_task()
{
    const auto almanac = read_almanac();

    auto seeds = almanac.seeds
        | std::views::chunk(2)
        | std::views::transform([](const auto idx_pair) {
              return std::views::iota(idx_pair.front(), idx_pair.front() + idx_pair.back());
          })
        | std::views::join;

    return solve_for_seeds(seeds, almanac.mappings);
}
}