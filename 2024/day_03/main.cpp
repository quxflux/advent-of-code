#include <util.hpp>

#include <optional>
#include <regex>
#include <utility>

namespace aoc {
namespace {
    enum class token {
        mul,
        do_it,
        dont_do_it
    };

    std::optional<std::pair<token, size_t>> find_next(std::string_view s)
    {
        constexpr auto tokens = //
            std::to_array<std::pair<token, std::string_view>>( //
                {
                    { token::mul, "mul" },
                    { token::do_it, "do()" },
                    { token::dont_do_it, "don't()" } });

        std::optional<std::pair<token, size_t>> min_found;

        for (const auto [tok, str] : tokens) {
            auto pos = s.find(str);

            if (pos == std::string::npos)
                continue;

            pos += str.length();

            if (!min_found || pos < min_found->second)
                min_found = { tok, pos };
        }

        return min_found;
    }
}

size_t first_task()
{

    const std::regex r(R"(\((\d+),(\d+)\))");

    size_t sum = 0;
    bool is_active = true;

    for (auto t : load_input_by_line()) {
        while (!t.empty()) {

            if (const auto next_token = find_next(t); !next_token) {
                break;
            } else {
                t = t.substr(next_token->second);

                if (next_token->first == token::do_it || next_token->first == token::dont_do_it) {
                    is_active = next_token->first == token::do_it;
                    continue;
                }
            }

            const std::string s = [&] {
                if (const auto next_after_token = find_next(t); next_after_token.has_value())
                    return std::string { t.substr(0, next_after_token->second) };

                return std::string { t };
            }();

            std::smatch match;
            if (std::regex_search(s, match, r)) {
                const auto a = to_int(match[1].str());
                const auto b = to_int(match[2].str());

                sum += a * b;

                t = t.substr(match[0].length());
            }
        }
    }

    return sum;
}

size_t second_task()
{
    return 0;
}
}
