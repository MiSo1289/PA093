#pragma once

#include <cmath>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <pa093/algorithm/constants.hpp>

namespace pa093::algorithm
{

[[nodiscard]] inline auto
circumcircle_center(glm::vec2 const a,
                    glm::vec2 const b,
                    glm::vec2 const c) noexcept -> std::optional<glm::vec2>
{
    auto const a_sq = glm::length2(a);
    auto const b_sq = glm::length2(b);
    auto const c_sq = glm::length2(c);

    auto const det = [&]
    {
        auto m = glm::mat2{};
        m[0] = b - a;
        m[1] = c - a;
        return glm::determinant(m);
    }();

    if (std::abs(det) < constants::epsilon_determinant)
    {
        // Collinear points
        return std::nullopt;
    }

    return glm::vec2{
        a_sq * (b.y - c.y) + b_sq * (c.y - a.y) + c_sq * (a.y - b.y),
        a_sq * (c.x - b.x) + b_sq * (a.x - c.x) + c_sq * (b.x - a.x),
    } / (2.0f * det);
}

} // namespace pa093::algorithm
