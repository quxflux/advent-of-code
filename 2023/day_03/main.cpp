#include <util.hpp>

#include <map>

#include <mdspan> // works only with VS22 preview 17.9

namespace aoc {
namespace {
    template <typename T>
    struct arr2d {
        arr2d() = default;

        arr2d(const size_t rows, const size_t cols)
            : storage(rows * cols)
            , mdspan(storage.data(), rows, cols)
        {
        }

        std::vector<T> storage;
        std::mdspan<T, std::extents<size_t, std::dynamic_extent, std::dynamic_extent>> mdspan;
    };

    auto row_span(auto& arr, const ptrdiff_t row)
    {
        return std::span { &arr.mdspan[std::array<ptrdiff_t, 2> { row, 0 }], arr.mdspan.extent(1) };
    }

    auto index_view(auto& arr)
    {
        return std::views::cartesian_product( //
            std::views::iota(ptrdiff_t { 0 }, static_cast<ptrdiff_t>(arr.mdspan.extent(0))), //
            std::views::iota(ptrdiff_t { 0 }, static_cast<ptrdiff_t>(arr.mdspan.extent(1))));
    }

    auto neighbor_indices_view(const ptrdiff_t size)
    {
        return std::views::cartesian_product( //
            std::views::iota(-size / 2, size / 2 + 1), //
            std::views::iota(-size / 2, size / 2 + 1));
    }

    enum class field {
        empty,
        digit,
        sign
    };

    field classify(const char c)
    {
        if (c == '.')
            return field::empty;
        if (quxflux::is_digit(c))
            return field::digit;

        return field::sign;
    }

    arr2d<char> read_input()
    {
        auto lines = std::views::split(QUXFLUX_GET_INPUT(), '\n') | std::views::transform(quxflux::as_string_view);
        const auto height = std::ranges::distance(lines.begin(), lines.end());
        const auto width = std::ranges::size(lines.front());

        arr2d<char> map { static_cast<size_t>(height), static_cast<size_t>(width) };

        for (const auto [idx, line] : std::views::enumerate(lines))
            std::ranges::copy(line, row_span(map, idx).begin());

        return map;
    }

    arr2d<field> classify_map(const arr2d<char>& map)
    {
        arr2d<field> classified_map { map.mdspan.extent(0), map.mdspan.extent(1) };
        std::ranges::transform(map.storage, classified_map.storage.begin(), &classify);
        return classified_map;
    }

    arr2d<char> make_dilated_mask(const size_t rows, const size_t cols, auto&& seed_pixels)
    {
        arr2d<char> mask { rows, cols };

        for (const auto [row, col] : seed_pixels) {
            for (const auto [delta_row, delta_col] : neighbor_indices_view(3)) {
                const auto offset_row = row + delta_row;
                const auto offset_col = col + delta_col;

                if (offset_row < 0 || offset_row >= rows || offset_col < 0 || offset_col >= cols)
                    continue;

                mask.mdspan[std::array { offset_row, offset_col }] = 1;
            }
        }

        return mask;
    }
}

size_t first_task()
{
    const auto map = read_input();
    const auto rows = map.mdspan.extent(0);
    const auto cols = map.mdspan.extent(1);

    const auto classified_map = classify_map(map);
    const auto dilated_mask = make_dilated_mask(rows, cols, index_view(classified_map) | std::views::filter([&](const auto indices) {
        const auto [row, col] = indices;
        return classified_map.mdspan[std::array { row,
                   col }]
            == field::sign;
    }));

    size_t sum = 0;

    for (const auto row_idx : std::views::iota(ptrdiff_t { 0 }, static_cast<ptrdiff_t>(map.mdspan.extent(0)))) {
        const auto classified_row = row_span(classified_map, row_idx);
        const auto map_row = row_span(map, row_idx);

        for (const auto& t : std::views::enumerate(classified_row) | std::views::chunk_by([](const auto first, const auto second) { return std::get<1>(first) == std::get<1>(second); })) {

            if (std::get<1>(t.front()) != field::digit)
                continue;

            if (std::ranges::none_of(t | std::views::elements<0>, [=](const auto col_idx) {
                    return dilated_mask.mdspan[std::array { row_idx, ptrdiff_t { col_idx } }] != 0;
                }))
                continue;

            const std::span data = map_row.subspan(std::get<0>(t.front()),
                std::get<0>(t.back()) - std::get<0>(t.front()) + 1);
            sum += std::stoul(std::string { data.begin(), data.end() });
        }
    }

    return sum;
}

size_t second_task()
{
    const auto map = read_input();
    const auto rows = map.mdspan.extent(0);
    const auto cols = map.mdspan.extent(1);

    const auto classified_map = classify_map(map);

    using segment_id = uint16_t;
    using part_value = uint16_t;

    struct segment {
        segment_id segment_id;
        part_value value;
    };

    auto segmented_map = arr2d<segment> { rows, cols };

    {
        uint16_t cnt = 0;
        for (const auto row_idx : std::views::iota(ptrdiff_t { 0 }, static_cast<ptrdiff_t>(map.mdspan.extent(0)))) {
            const auto classified_row = row_span(classified_map, row_idx);
            const auto map_row = row_span(map, row_idx);

            for (const auto& t : std::views::enumerate(classified_row) | std::views::chunk_by([](const auto first, const auto second) {
                     return std::get<1>(first) == std::get<1>(second);
                 })) {

                if (std::get<1>(t.front()) != field::digit)
                    continue;

                const auto label = ++cnt;

                const std::span data = map_row.subspan(std::get<0>(t.front()),
                    std::get<0>(t.back()) - std::get<0>(t.front()) + 1);
                const auto value = std::stoul(std::string { data.begin(), data.end() });

                for (const auto col_idx : t | std::views::keys)
                    segmented_map.mdspan[std::array { row_idx,
                        col_idx }]
                        = { .segment_id = label, .value = static_cast<part_value>(value) };
            }
        }
    }

    auto gear_ratios = index_view(map) //
        | std::views::transform([&](const auto idx) -> size_t {
              const auto [row, col] = idx;

              if (map.mdspan[std::array { row, col }] != '*')
                  return 0;

              std::map<segment_id, part_value> adjacent_labels;

              for (const auto [delta_row, delta_col] : neighbor_indices_view(3)) {
                  const auto offset_row = row + delta_row;
                  const auto offset_col = col + delta_col;

                  if (offset_row < 0 || offset_row >= map.mdspan.extent(0) || offset_col < 0 || offset_col >= map.mdspan.extent(1))
                      continue;

                  const auto segment = segmented_map.mdspan[std::array { offset_row, offset_col }];
                  if (segment.segment_id != 0)
                      adjacent_labels[segment.segment_id] = segment.value;
              }

              if (adjacent_labels.size() != 2)
                  return 0;

              return adjacent_labels.begin()->second * adjacent_labels.rbegin()->second;
          });

    return std::ranges::fold_left(gear_ratios, size_t { 0 }, std::plus {});
}
}