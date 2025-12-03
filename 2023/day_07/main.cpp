#include <util.hpp>

#include <utility>

namespace aoc {
namespace {
    enum class card_label {
        d2,
        d3,
        d4,
        d5,
        d6,
        d7,
        d8,
        d9,
        T,
        J,
        Q,
        K,
        A
    };

    constexpr size_t num_labels = 13;

    using hand = std::array<card_label, 5>;

    constexpr card_label convert(const char c)
    {
        constexpr std::array vals { '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A' };
        return static_cast<card_label>(std::ranges::distance(vals.begin(), std::ranges::find(vals, c)));
    }

    constexpr std::array<uint8_t, num_labels> histogram(const hand& hand)
    {
        std::array<uint8_t, num_labels> result {};

        for (const auto t : hand)
            result[std::to_underlying(t)]++;

        return result;
    }

    enum class special_rules {
        none,
        joker
    };

    template <special_rules Rules>
    constexpr uint8_t hand_strength(const hand& hand)
    {
        auto [hist, num_jokers] = [&] {
            auto t = histogram(hand);

            if constexpr (Rules == special_rules::joker) {
                std::array<uint8_t, num_labels - 1> hist_without_joker {};
                std::ranges::copy(std::span { t }.subspan(0, 9), hist_without_joker.begin());
                std::ranges::copy(std::span { t }.subspan(10), hist_without_joker.begin() + 9);

                return std::pair { hist_without_joker, t[std::to_underlying(card_label::J)] };
            } else {
                return std::pair { t, uint8_t { 0 } };
            }
        }();
        std::ranges::sort(hist, std::greater {});

        if (const bool is_five_of_a_kind = hist[0] + num_jokers == 5)
            return 6;
        if (const bool is_four_of_a_kind = hist[0] + num_jokers == 4)
            return 5;
        if (const bool is_full_house = hist[0] + num_jokers == 3 && hist[1] == 2)
            return 4;
        if (const bool is_three_of_a_kind = hist[0] + num_jokers == 3)
            return 3;
        if (const bool is_two_pair = hist[0] + num_jokers == 2 && hist[1] == 2)
            return 2;
        if (const bool is_one_pair = hist[0] + num_jokers == 2)
            return 1;

        return 0;
    }

    template <special_rules Rules>
    constexpr bool compare_hands(const hand& lhs, const hand& rhs)
    {
        const auto lhs_strength = hand_strength<Rules>(lhs);
        const auto rhs_strength = hand_strength<Rules>(rhs);

        if (lhs_strength != rhs_strength)
            return lhs_strength < rhs_strength;

        constexpr auto card_strength = [](const card_label l) {
            if constexpr (Rules == special_rules::joker) {
                if (l == card_label::J)
                    return -1;
            }

            return std::to_underlying(l);
        };

        const auto zipped = std::views::zip(lhs, rhs);
        const auto idx = std::ranges::distance(zipped.begin(), std::ranges::find_if(zipped, [](const auto labels) {
            return std::apply(std::not_equal_to {}, labels);
        }));

        return card_strength(lhs[idx]) < card_strength(rhs[idx]);
    }

    auto read_input()
    {
        return std::vector { std::from_range, load_input_by_line() | std::views::transform([](const std::string_view line) {
                                hand h;
                                std::ranges::transform(line.substr(0, 5), h.begin(), &convert);
                                return std::pair { h, std::stoul(std::string { line.substr(6) }) };
                            }) };
    }

    template <special_rules Rules>
    size_t calculate()
    {
        auto data = read_input();
        std::ranges::sort(data, &compare_hands<Rules>, [](const auto& p) { return p.first; });

        return std::ranges::fold_left(std::views::enumerate(data | std::views::elements<1>) | std::views::transform([](const auto rank_and_bid) {
            const auto [rank, bid] = rank_and_bid;
            return (rank + 1) * bid;
        }),
            size_t { 0 }, std::plus {});
    }
}

size_t first_task()
{
    return calculate<special_rules::none>();
}

size_t second_task()
{
    return calculate<special_rules::joker>();
}

}
