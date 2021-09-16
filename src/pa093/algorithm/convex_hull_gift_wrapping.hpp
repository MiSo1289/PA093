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
#include <spdlog/spdlog.h>

namespace pa093::algorithm {

template<std::forward_iterator In, std::output_iterator<glm::vec2> Out>
Out
convex_hull_gift_wrapping(
    In const in_begin,
    std::sentinel_for<In> auto const in_end,
    Out out) requires std::same_as<std::iter_value_t<In>, glm::vec2>
{
    static constexpr auto epsilon = 0.0001f;

    spdlog::debug("Running convex hull (gift wrapping)");

    if (in_begin == in_end) {
        return out;
    }

    auto const start =
        std::ranges::max_element(in_begin, in_end, std::less{}, &glm::vec2::y);
    auto curr = start;
    auto last_point = *curr - glm::vec2{ 1.0f, 0.0f };

    do {
        *out++ = *curr;

        auto last_dir = glm::normalize(*curr - last_point);
        last_point = *curr;

        spdlog::debug("Current point {0} = ({1}, {2})",
                      std::distance(in_begin, curr),
                      curr->x,
                      curr->y);
        spdlog::debug("Last dir ({0}, {1})", last_dir.x, last_dir.y);
        curr = std::ranges::min_element(
            in_begin, in_end, std::less{}, [&](glm::vec2 const point) {
                spdlog::debug("Examining point ({0}, {1})", point.x, point.y);

                auto dir = point - *curr;
                if (glm::length2(dir) < epsilon) {
                    spdlog::debug("Same or almost same as previous point");
                    return std::numeric_limits<float>::max();
                }

                dir = glm::normalize(dir);
                auto const angle = glm::angle(last_dir, dir);
                spdlog::debug(
                    "Angle with dir ({0}, {1}) is {2}", dir.x, dir.y, angle);

                return angle;
            });
    } while (curr != start);

    return out;
}

} // namespace pa093::algorithm
