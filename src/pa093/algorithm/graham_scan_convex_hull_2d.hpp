#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <ranges>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <pa093/algorithm/constants.hpp>

namespace pa093::algorithm
{

class GrahamScanConvexHull2d
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
        if (first == last)
        {
            return result;
        }

        points_.clear();
        std::ranges::copy(first, last, std::back_inserter(points_));

        // Find pivot point
        auto const pivot_iter =
            std::ranges::min_element(points_, std::less{}, &glm::vec2::y);
        auto const pivot = *pivot_iter;

        // Move it to the start of the in-place stack
        std::iter_swap(points_.begin(), pivot_iter);

        // Remove points packed within the epsilon radius of the pivot, as the
        // angle with the pivot cannot be determined.
        auto const removed_points = std::ranges::remove_if(
            std::next(points_.begin()),
            points_.end(),
            [&](glm::vec2 const point)
            {
                return glm::all(glm::epsilonEqual(
                    point, pivot, constants::epsilon_distance));
            });
        points_.erase(removed_points.begin(), removed_points.end());

        // Sort remaining points by the cosine of the angle of X axis and vector
        // from the pivot, descending.
        // This is the same as sorting by the actual angle ascending as it is
        // always in the range [0, pi], since the pivot is the min-Y point.
        std::ranges::sort(std::next(points_.begin()),
                          points_.end(),
                          std::greater{},
                          [&](glm::vec2 const point) {
                              return glm::normalize(point - pivot).x;
                          });

        // Repeat the pivot at the end of the processed sequence, so that any
        // right turns at the end get removed by the processing loop.
        points_.push_back(pivot);

        // Use an in-place stack in the already processed part of the sequence
        auto stack_top = points_.begin();

        // Process remaining point in order
        for (auto const curr_point : points_)
        {
            *stack_top++ = curr_point;

            // Backtrack and remove right turns
            while (std::distance(points_.begin(), stack_top) >= 3)
            {
                auto const point_c = std::prev(stack_top);
                auto const point_b = std::prev(point_c);
                auto const point_a = std::prev(point_b);

                auto const cross =
                    (point_b->x - point_a->x) * (point_c->y - point_a->y) -
                    (point_b->y - point_a->y) * (point_c->x - point_a->x);

                if (cross < 0)
                {
                    // Right turn, remove middle point
                    *point_b = *point_c;
                    --stack_top;
                }
                else
                {
                    // Left turn, stop backtracking
                    break;
                }
            }
        }

        // The last point left on the stack will be the repeated pivot;
        // remove it.
        --stack_top;

        // Output the final stack contents
        return std::ranges::copy(points_.begin(), stack_top, result).out;
    }

private:
    std::vector<glm::vec2> points_;
};

} // namespace pa093::algorithm
