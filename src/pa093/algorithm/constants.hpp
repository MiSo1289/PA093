#pragma once

namespace pa093::algorithm::constants
{

/**
 * Distance at which two points are considered equal
 */
inline constexpr auto epsilon_distance = 1e-8f;
/**
 * Matrices with absolute determinant smaller than this are considered singular
 */
inline constexpr auto epsilon_determinant = 1e-8f;

} // namespace pa093::algorithm::constants
