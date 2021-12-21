#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>

#include <pa093/algorithm/constants.hpp>
#include <pa093/algorithm/geometric_functions.hpp>

namespace pa093::algorithm::triangulation
{

class DualGraph
{
public:
    [[nodiscard]] explicit DualGraph(float const hull_edge_length) noexcept
        : hull_edge_length_{ hull_edge_length }
    {
    }

    template<std::ranges::input_range R, std::output_iterator<glm::vec2> O>
    requires std::same_as<std::ranges::range_value_t<R>, glm::vec2>
    auto operator()(R&& range, O const result) -> O
    {
        return (*this)(
            std::ranges::begin(range), std::ranges::end(range), result);
    }

    template<std::input_iterator I,
             std::sentinel_for<I> S,
             std::output_iterator<glm::vec2> O>
    requires std::same_as<std::iter_value_t<I>, glm::vec2>
    auto operator()(I const first, S const last, O result) -> O
    {
        reset();

        // Collect triangles from input range
        {
            auto triangle = triangle_type{};
            auto i = 0u;

            for (auto const point : std::ranges::subrange{ first, last })
            {
                triangle[i++] = point;

                if (i == 3u)
                {
                    triangles_.push_back(triangle);
                    i = 0u;
                }
            }
        }

        // Compute the dual graph vertices as the circumcircle center
        // for each triangle.
        for (auto const [p1, p2, p3] : triangles_)
        {
            if (auto const s = circumcircle_center(p1, p2, p3))
            {
                dual_vertices_.push_back(*s);
            }
            else
            {
                // Collinear points - input triangulation is degenerate;
                // use mean instead as a work-around
                dual_vertices_.push_back((p1 + p2 + p3) / 3.0f);
            }
        }

        hull_edges_mask_.assign(triangles_.size() * 3u, true);

        // Find triangle adjacencies
        for (auto const [i, t1] : ranges::views::enumerate(triangles_))
        {
            for (auto const [j, t2] : ranges::views::enumerate(triangles_) |
                                          ranges::views::drop(i + 1u))
            {
                if (auto const adj = find_adjacency(t1, t2))
                {
                    // Output edge in the dual graph
                    *result++ = dual_vertices_[i];
                    *result++ = dual_vertices_[j];

                    auto const [k, l] = *adj;
                    // Mark triangle edges as internal
                    hull_edges_mask_[i * 3u + k] = false;
                    hull_edges_mask_[j * 3u + l] = false;
                }
            }
        }

        // Output hull edges
        for (auto const [i, t] : ranges::views::enumerate(triangles_))
        {
            for (auto const j : std::views::iota(0u, 3u))
            {
                if (hull_edges_mask_[i * 3u + j])
                {
                    auto const p1 = t[j];
                    auto const p2 = t[(j + 1u) % 3u];
                    // Counter-clockwise edge vector
                    auto const v = p2 - p1;
                    auto const l = glm::length(v);

                    if (l <= constants::epsilon_distance)
                    {
                        // Skip degenerate edge
                        continue;
                    }

                    // Rotate by -pi / 2 to obtain outward vector
                    auto const n = glm::vec2{ v.y, -v.x } / l;

                    auto const s = dual_vertices_[i];
                    // Output dual hull edge
                    *result++ = s;
                    *result++ = s + hull_edge_length_ * n;
                }
            }
        }

        return result;
    }

    void reset()
    {
        triangles_.clear();
        dual_vertices_.clear();
        hull_edges_mask_.clear();
    }

private:
    using triangle_type = std::array<glm::vec2, 3u>;
    using edge_index_type = std::size_t;

    float hull_edge_length_;
    std::vector<triangle_type> triangles_;
    std::vector<glm::vec2> dual_vertices_;
    std::vector<bool> hull_edges_mask_;

    [[nodiscard]] static auto find_adjacency(triangle_type const& t1,
                                             triangle_type const& t2) noexcept
        -> std::optional<std::array<edge_index_type, 2u>>
    {
        for (auto const k : std::views::iota(0u, 3u))
        {
            auto const e1 = std::array{ t1[k], t1[(k + 1u) % 3u] };

            for (auto const l : std::views::iota(0u, 3u))
            {
                auto const e2 = std::array{ t2[(l + 1u) % 3u], t2[l] };

                if (e1 == e2)
                {
                    return std::array<std::size_t, 2u>{ k, l };
                }
            }
        }

        return std::nullopt;
    }
};

} // namespace pa093::algorithm::triangulation
