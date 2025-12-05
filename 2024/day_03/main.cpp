#include <util.hpp>

#include <optional>
#include <regex>
#include <utility>

namespace aoc {
namespace {
    enum class token_t {
        mul,
        do_it,
        dont_do_it
    };

    struct match {
        token_t token;
        size_t start;
        size_t end;
    };

    std::optional<match> find_next(std::string_view s, const auto& tokens)
    {
        std::optional<match> min_found;

        for (const auto [tok, str] : tokens) {
            auto pos = s.find(str);

            if (pos == std::string::npos)
                continue;

            if (!min_found || pos + str.length() < min_found->end)
                min_found = { tok, pos, pos + str.length() };
        }

        return min_found;
    }
}

size_t first_task()
{
    const std::regex r(R"(mul\((\d+),(\d+)\))");

    size_t sum = 0;

    for (auto t : load_input_by_line()) {
        while (!t.empty()) {
            auto pos = t.find("mul");
            if (pos == std::string::npos)
                break;
            t = t.substr(pos);

            pos = t.find(')');
            if (pos == std::string::npos)
                break;

            const auto s = std::string { t.substr(0, pos + 1) };

            t = t.substr(pos + 1);

            std::smatch match;
            if (std::regex_search(s, match, r)) {
                const auto a = to_int(match[1].str());
                const auto b = to_int(match[2].str());
                sum += a * b;
            }
        }
    }

    return sum;
}

size_t second_task()
{
    const std::regex r(R"(mul\((\d+),(\d+)\))");

    size_t sum = 0;

    constexpr auto tokens = std::to_array<std::pair<token_t, std::string_view>>({
        { token_t::do_it, "do()" },
        { token_t::mul, "mul(" },
        { token_t::dont_do_it, "don't()" },
    });

    bool active = true;
    for (auto t : load_input_by_line()) {
        while (!t.empty()) {

            const auto next = find_next(t, tokens);
            if (!next)
                break;

            if (next->token == token_t::do_it || next->token == token_t::dont_do_it) {
                active = next->token == token_t::do_it;
                t = t.substr(next->end);
                continue;
            }

            t = t.substr(next->start);

            auto end_pos = t.find(')');
            if (end_pos == std::string::npos)
                break;

            end_pos += 1;
            const auto s = std::string { t.substr(0, end_pos) };
            std::smatch match;
            if (std::regex_search(s, match, r)) {
                const auto a = to_int(match[1].str());
                const auto b = to_int(match[2].str());
                if (active)
                    sum += a * b;
            }
            t = t.substr(end_pos);
        }
    }

    return sum;
}
}
