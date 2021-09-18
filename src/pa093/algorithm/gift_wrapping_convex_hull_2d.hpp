#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <ranges>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace pa093::algorithm
{

class GiftWrappingConvexHull2d
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
        static constexpr auto epsilon = 1e-8f;

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

            auto last_dir = *curr - prev_point;
            if (glm::length2(last_dir) < epsilon)
            {
                // Degenerate case:
                // All points are packed within the epsilon radius.
                break;
            }

            last_dir = glm::normalize(last_dir);
            prev_point = *curr;

            curr = std::ranges::min_element(
                first,
                last,
                std::less{},
                [&](glm::vec2 const point)
                {
                    auto dir = point - *curr;
                    if (glm::length2(dir) < epsilon)
                    {
                        return std::numeric_limits<float>::infinity();
                    }

                    dir = glm::normalize(dir);
                    return glm::angle(last_dir, dir);
                });
        } while (curr != start);

        return result;
    }
};

} // namespace pa093::algorithm
