#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <ranges>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "pa093/algorithm/constants.hpp"

namespace pa093::algorithm::convex_hull
{

class GiftWrapping
{
public:
    template<std::ranges::forward_range R, std::output_iterator<glm::vec2> O>
    requires std::same_as<std::ranges::range_value_t<R>, glm::vec2>
    auto operator()(R&& range, O const result) -> O
    {
        return (*this)(
            std::ranges::begin(range), std::ranges::end(range), result);
    }

    template<std::forward_iterator I,
             std::sentinel_for<I> S,
             std::output_iterator<glm::vec2> O>
    requires std::same_as<std::iter_value_t<I>, glm::vec2>
    auto operator()(I const first, S const last, O result) -> O
    {
        if (first == last)
        {
            return result;
        }

        auto const start =
            std::ranges::max_element(first, last, std::less{}, &glm::vec2::y);
        auto curr = start;
        auto prev_point = *curr - glm::vec2{ 1.0f, 0.0f };

        do
        {
            *result++ = *curr;

            if (glm::all(glm::epsilonEqual(
                    prev_point, *curr, constants::epsilon_distance)))
            {
                // Degenerate case:
                // All points are packed within the epsilon radius.
                break;
            }

            auto const last_dir = glm::normalize(*curr - prev_point);
            prev_point = *curr;

            curr = std::ranges::min_element(
                first,
                last,
                std::less{},
                [&](glm::vec2 const point)
                {
                    if (glm::all(glm::epsilonEqual(
                            point, *curr, constants::epsilon_distance)))
                    {
                        return std::numeric_limits<float>::infinity();
                    }

                    return glm::angle(last_dir, glm::normalize(point - *curr));
                });
        } while (curr != start);

        return result;
    }
};

} // namespace pa093::algorithm
