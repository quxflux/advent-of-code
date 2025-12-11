#include <array>
#include <cassert>
#include <execution>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <util.hpp>

namespace aoc {
namespace {
    using value_type = int32_t;
    using point = std::array<value_type, 3>;
    using distance = float;

    std::vector<point> load_data()
    {
        return load_input_by_line() | std::views::transform([](const std::string_view line) {
            point p;
            std::sscanf(std::string(line).c_str(), "%ld,%ld,%ld", &p[0], &p[1], &p[2]);
            return p;
        })
            | std::ranges::to<std::vector>();
    }

    distance distance_squared(const point& a, const point& b)
    {
        distance dist = 0;
        for (size_t i = 0; i < 3; ++i) {
            const auto diff = static_cast<distance>(a[i]) - static_cast<distance>(b[i]);
            dist += diff * diff;
        }
        return dist;
    }

    struct kd_tree {

        kd_tree(std::span<const point> pts, const size_t leaf_threshold_size = 10)
            : points_(std::move(pts))
            , point_indices_(std::views::iota(size_t { 0 }, points_.size()) | std::ranges::to<std::vector>())
        {
            nodes_.emplace_back();
            build_tree(0, 0, points_.size(), leaf_threshold_size);
        }

        std::span<const point> points() const
        {
            return std::span { points_ };
        }

        template <std::invocable<size_t> CandidateFilter = decltype([](size_t) { return true; })>
        std::optional<size_t> closest_point(const size_t query_idx, const CandidateFilter& candidate_filter = {}) const
        {
            const point& query = points_[query_idx];

            std::optional<size_t> closest {};
            distance closest_dist = std::numeric_limits<value_type>::max();

            [&](this auto&& self, const size_t node_idx) -> void {
                const node& n = nodes_[node_idx];

                const bool is_leaf = (n.left == node::no_child || n.right == node::no_child);

                if (is_leaf) {
                    assert(n.left == node::no_child && n.right == node::no_child);

                    for (const size_t point_idx : std::span { point_indices_.begin() + n.start, n.len }) {
                        if (!candidate_filter(point_idx))
                            continue;

                        const auto dist = distance_squared(query, points_[point_idx]);
                        if (dist < closest_dist) {
                            closest_dist = dist;
                            closest = point_idx;
                        }
                    }

                    return;
                }

                const auto diff = static_cast<distance>(query[n.split_dim]) - static_cast<distance>(n.split_value);

                if (diff <= 0) {
                    self(n.left);
                    if (diff * diff <= closest_dist)
                        self(n.right);
                } else {
                    self(n.right);
                    if (diff * diff <= closest_dist)
                        self(n.left);
                }
            }(0);

            return closest;
        }

    private:
        struct node {
            uint8_t split_dim = 0;
            value_type split_value = 0;

            static constexpr auto no_child = std::numeric_limits<size_t>::max();

            size_t left = no_child;
            size_t right = no_child;
            size_t start = 0;
            size_t len = 0;
        };

        void build_tree(const size_t node_idx, const size_t start, const size_t len, const size_t leaf_threshold_size)
        {
            if (len == 0)
                return;

            const std::span this_tree_point_indices { point_indices_.begin() + start, len };
            const auto get_point = [this](size_t idx) { return points_[idx]; };

            const point extents = [=] {
                point min {};
                point max {};
                std::ranges::fill(min, std::numeric_limits<value_type>::max());

                for (const auto& point : this_tree_point_indices | std::views::transform(get_point)) {
                    for (size_t i = 0; i < 3; ++i) {
                        min[i] = std::min(min[i], point[i]);
                        max[i] = std::max(max[i], point[i]);
                    }
                }

                point extents {};
                for (size_t i = 0; i < 3; ++i)
                    extents[i] = max[i] - min[i];

                return extents;
            }();

            const auto [max_dim, _] = std::ranges::max(std::views::enumerate(extents), std::less {}, [](const auto& idx_and_extent) { return std::get<1>(idx_and_extent); });

            std::ranges::nth_element(this_tree_point_indices.begin(),
                std::next(this_tree_point_indices.begin(), len / 2),
                this_tree_point_indices.end(),
                std::less {},
                [&](const size_t point_idx) { return get_point(point_idx)[max_dim]; });

            {
                node& n = nodes_[node_idx];
                n.start = start;
                n.len = len;
                n.split_dim = max_dim;
                n.split_value = get_point(this_tree_point_indices[len / 2])[max_dim];
            }

            if (len <= leaf_threshold_size)
                return;

            const size_t mid = start + len / 2;
            nodes_.emplace_back();
            nodes_.emplace_back();
            const auto left_child_idx = nodes_.size() - 2;
            const auto right_child_idx = nodes_.size() - 1;

            build_tree(nodes_[node_idx].left = left_child_idx, start, mid - start, leaf_threshold_size);
            build_tree(nodes_[node_idx].right = right_child_idx, mid, start + len - mid, leaf_threshold_size);
        }

        std::span<const point> points_;
        std::vector<size_t> point_indices_;
        std::vector<node> nodes_;
    };
}

class solver {
public:
    solver(std::vector<point> points)
        : points_(std::move(points))
        , edge_black_listed_(max_edge_index(points_.size() + 1), false)
        , edge_is_direct_connection_(max_edge_index(points_.size() + 1), false)
        , point_to_circuit_(points_.size())
    {
        for (size_t idx = 0; idx < points_.size(); ++idx) {
            point_to_circuit_[idx] = circuits_.size();
            circuits_.emplace_back().insert(idx);
        }
    }

    struct edge {
        edge(const size_t& a_, const size_t& b_)
            : a(std::min(a_, b_))
            , b(std::max(a_, b_))
        {
        }

        size_t a;
        size_t b;

        friend bool operator==(const edge& lhs, const edge& rhs) = default;
    };

    std::span<const point> points() const
    {
        return std::span { points_ };
    }

    std::optional<edge> solve(std::optional<size_t> max_steps = std::nullopt)
    {
        std::optional<edge> last_added_edge;

        size_t i = 0;
        for (; !max_steps || i < *max_steps; ++i) {

            std::optional<std::pair<size_t, size_t>> closest;
            distance closest_dist = std::numeric_limits<value_type>::max();
            std::mutex mtx;

            const auto chunked = std::views::enumerate(points_) | std::views::chunk(10);
            std::for_each(std::execution::par_unseq, chunked.begin(), chunked.end(), [&, this](const auto& idx_and_p) {
                thread_local std::optional<std::pair<size_t, size_t>> thread_closest;
                thread_local distance thread_closest_dist = std::numeric_limits<value_type>::max();

                for (const auto& [p_idx, p] : idx_and_p) {

                    const auto this_closest = tree_.closest_point(p_idx, [&](const size_t candidate) {
                        if (candidate == p_idx)
                            return false;

                        const auto edge_pair = edge { static_cast<size_t>(p_idx), candidate };
                        const auto edge_idx = pairing_func(edge_pair);
                        return !edge_is_direct_connection_[edge_idx] && !edge_black_listed_[edge_idx];
                    });

                    if (this_closest) {
                        const auto this_dist = distance_squared(p, points_[*this_closest]);
                        if (!thread_closest || this_dist < thread_closest_dist) {
                            thread_closest = { p_idx, *this_closest };
                            thread_closest_dist = this_dist;
                        }
                    }
                }

                std::unique_lock lock { mtx };
                if (thread_closest && thread_closest_dist < closest_dist) {
                    closest = thread_closest;
                    closest_dist = thread_closest_dist;
                }
            });

            if (!closest)
                break;

            if (max_steps) {
                if (i % ((*max_steps) / 100) == 0)
                    std::cout << progress { static_cast<float>(i) / static_cast<float>(*max_steps) };
            } else {
                if (i % 100 == 0)
                    std::cout << "Iteration  " << i << '\r';
            }

            const auto& [a, b] = *closest;
            const auto edge_pair = edge { a, b };
            const auto edge_idx = pairing_func(edge_pair);

            if (share_circuit(a, b)) {
                edge_black_listed_[edge_idx] = true;
                continue;
            }

            edge_is_direct_connection_[edge_idx] = true;
            last_added_edge = edge_pair;
            merge_circuits(point_to_circuit_[a], point_to_circuit_[b]);
        }

        return last_added_edge;
    }

    std::array<size_t, 3> top_three_circuit_sizes() const
    {
        auto circuit_sizes = circuits_ //
            | std::views::filter(std::not_fn(&std::unordered_set<size_t>::empty)) //
            | std::views::transform(&std::unordered_set<size_t>::size) //
            | std::ranges::to<std::vector>();
        std::ranges::sort(circuit_sizes, std::greater {});
        std::array<size_t, 3> top_three {};
        std::ranges::copy(circuit_sizes | std::views::take(3), top_three.begin());
        return top_three;
    }

private:
    struct edge_traits {
        bool is_direct_connection = false;
        bool is_blacklisted = false;
    };

    constexpr size_t pairing_func(const edge& pp) const
    {
        return pp.b * pp.b + pp.a;
    }

    constexpr size_t max_edge_index(const size_t n_points) const
    {
        return pairing_func({ n_points, n_points - 1 });
    }

    bool share_circuit(const size_t& a, const size_t& b) const
    {
        return a == b || point_to_circuit_[a] == point_to_circuit_[b];
    }

    void merge_circuits(size_t idx_a, size_t idx_b)
    {
        assert(idx_a != idx_b);

        if (circuits_[idx_a].size() < circuits_[idx_b].size())
            std::swap(idx_a, idx_b);
        for (const auto p_idx : circuits_[idx_b]) {
            circuits_[idx_a].insert(p_idx);
            point_to_circuit_[p_idx] = idx_a;
        }
        circuits_[idx_b].clear();
    }

    std::vector<point> points_;
    kd_tree tree_ { points_ };

    std::vector<size_t> point_to_circuit_;
    std::vector<std::unordered_set<size_t>> circuits_;
    std::vector<char> edge_black_listed_;
    std::vector<char> edge_is_direct_connection_;
};

size_t first_task() // 123420
{
    std::cout << "\n";

    solver solve(load_data());
    solve.solve(1000);
    std::cout << "\n";

    return std::ranges::fold_left_first(solve.top_three_circuit_sizes(), std::multiplies {}).value();
}

size_t second_task() // 673096646
{
    // took around 115320 iterations
    std::cout << "\n";

    solver solve(load_data());
    const auto last_added_ege = solve.solve();
    std::cout << "\n";

    return solve.points()[last_added_ege->a][0] * solve.points()[last_added_ege->b][0];
}
}