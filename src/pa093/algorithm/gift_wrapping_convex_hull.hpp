#pragma once

#include <algorithm>
#include <chrono>
#include <iterator>
#include <ranges>
#include <span>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace pa093::algorithm
{

class GiftWrappingConvexHull
{
public:
    template<std::forward_iterator In, std::output_iterator<glm::vec2> Out>
    requires std::same_as<std::iter_value_t<In>, glm::vec2>
    auto operator()(In const in_begin,
                    std::sentinel_for<In> auto const in_end,
                    Out out) -> Out
    {
        static constexpr auto epsilon = 0.0001f;

        if (in_begin == in_end)
        {
            return out;
        }

        auto const start = std::ranges::max_element(
            in_begin, in_end, std::less{}, &glm::vec2::y);
        auto curr = start;
        auto last_point = *curr - glm::vec2{ 1.0f, 0.0f };

        do
        {
            *out++ = *curr;

            auto last_dir = glm::normalize(*curr - last_point);
            last_point = *curr;

            curr = std::ranges::min_element(
                in_begin,
                in_end,
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

        return out;
    }
};

} // namespace pa093::algorithm
