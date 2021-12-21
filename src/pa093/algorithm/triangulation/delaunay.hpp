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

#include <pa093/algorithm/constants.hpp>
#include <pa093/algorithm/geometric_functions.hpp>

namespace pa093::algorithm::triangulation
{

[[nodiscard]] inline auto
delaunay_distance(glm::vec2 const p1,
                  glm::vec2 const p2,
                  glm::vec2 const p) noexcept -> float
{
    auto const s = circumcircle_center(p, p1, p2);
    if (not s)
    {
        // The center of the circumcircle is at infinity
        return std::numeric_limits<float>::infinity();
    }

    auto const r = glm::distance(p, *s);

    // If the angle (e1, p, e2) is < 90, return r, otherwise -r
    return std::copysign(r, glm::dot(p1 - p, p2 - p));
}

class Delaunay
{
public:
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

        std::ranges::copy(first, last, std::back_inserter(points_));

        if (points_.size() >= 3u)
        {
            // Form the initial triangle

            auto p1 = points_.front();
            // p2 is the closest point to p1
            auto p2 = *std::ranges::min_element(
                std::next(points_.begin()),
                points_.end(),
                std::less{},
                [&](glm::vec2 const p) { return glm::distance2(p1, p); });

            // p3 is the point that minimizes delaunay distance to (p1, p2)
            auto p3 = complete_triangle(p1, p2);
            if (not p3)
            {
                // (p1, p2) lies on the convex hull - no points to the left;
                // reverse the edge
                std::swap(p1, p2);
                p3 = complete_triangle(p1, p2);
            }

            if (p3)
            {
                active_boundary_.push_back({ p1, p2 });
                active_boundary_.push_back({ p2, *p3 });
                active_boundary_.push_back({ *p3, p1 });

                *result++ = p1;
                *result++ = p2;
                *result++ = *p3;
            }
        }

        while (not active_boundary_.empty())
        {
            auto const [p2, p1] = active_boundary_.back();
            active_boundary_.pop_back();

            if (auto const p3 = complete_triangle(p1, p2))
            {
                expand_active_boundary(p2, *p3);
                expand_active_boundary(*p3, p1);

                *result++ = p1;
                *result++ = p2;
                *result++ = *p3;
            }
        }

        return result;
    }

    void reset()
    {
        points_.clear();
        active_boundary_.clear();
    }

private:
    std::vector<glm::vec2> points_;
    std::vector<std::array<glm::vec2, 2u>> active_boundary_;

    [[nodiscard]] auto complete_triangle(glm::vec2 const p1,
                                         glm::vec2 const p2) noexcept
        -> std::optional<glm::vec2>
    {
        auto const left_points_end =
            std::ranges::partition(points_,
                                   [&](glm::vec2 const p)
                                   {
                                       auto m = glm::mat2{};
                                       m[0] = p2 - p1;
                                       m[1] = p - p1;
                                       return glm::determinant(m) >
                                              constants::epsilon_determinant;
                                   })
                .begin();

        auto const match = std::ranges::min_element(
            points_.begin(),
            left_points_end,
            std::less{},
            [&](glm::vec2 const p) { return delaunay_distance(p1, p2, p); });

        if (match == left_points_end)
        {
            return std::nullopt;
        }

        return *match;
    }

    void expand_active_boundary(glm::vec2 const p1, glm::vec2 const p2)
    {
        if (auto const reverse_match =
                std::ranges::find(active_boundary_, std::array{ p2, p1 });
            reverse_match != active_boundary_.end())
        {
            // Close the edge
            active_boundary_.erase(reverse_match);
        }
        else
        {
            active_boundary_.push_back({ p1, p2 });
        }
    }
};

} // namespace pa093::algorithm::triangulation
