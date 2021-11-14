#pragma once

#include <algorithm>
#include <deque>
#include <iterator>
#include <ranges>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace pa093::algorithm::triangulation
{

class SweepLine
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
        reset();

        if (std::ranges::distance(first, last) < 3)
        {
            return result;
        }

        // Copy the input
        std::ranges::copy(first, last, std::back_inserter(top_path_));

        // Find the extreme positions on the X axis
        auto const [leftmost, rightmost] =
            std::ranges::minmax_element(top_path_, std::less{}, &glm::vec2::x);
        auto const leftmost_index = std::distance(top_path_.begin(), leftmost);
        auto const rightmost_index =
            std::distance(top_path_.begin(), rightmost);

        auto const leftmost_point = *leftmost;

        // Separate the top and bottom paths between the extrema.
        // Leftmost point is not placed in either of the paths.
        if (leftmost < rightmost)
        {
            // Use front inserter to change the order of the second path
            // This way, both paths are ordered left-to-right
            std::ranges::copy(
                rightmost, top_path_.end(), std::front_inserter(bottom_path_));
            std::ranges::copy(
                top_path_.begin(), leftmost, std::front_inserter(bottom_path_));

            top_path_.erase(top_path_.begin(), std::next(leftmost));
            // Iterators are invalidated here, have to use indices
            top_path_.erase(
                std::next(top_path_.begin(), rightmost_index - leftmost_index),
                top_path_.end());
        }
        else
        {
            std::ranges::copy(std::next(leftmost),
                              top_path_.end(),
                              std::back_inserter(bottom_path_));
            std::ranges::copy(
                top_path_.begin(), rightmost, std::back_inserter(bottom_path_));

            top_path_.erase(top_path_.begin(), rightmost);
            top_path_.erase(
                std::next(top_path_.begin(), leftmost_index - rightmost_index),
                top_path_.end());

            std::ranges::reverse(top_path_);
        }

        if (not top_path_.empty() and not bottom_path_.empty())
        {
            // Check which path is really on the top, and swap them if needed.
            const auto top_vec =
                glm::normalize(top_path_.front() - leftmost_point);
            const auto bottom_vec =
                glm::normalize(bottom_path_.front() - leftmost_point);

            if (top_vec.y < bottom_vec.y)
            {
                std::swap(top_path_, bottom_path_);
            }
        }

        stack_.emplace_back(leftmost_point, Path::top);
        stack_.push_back(next_point());

        while (not paths_exhausted())
        {
            auto const [current_point, current_path] = next_point();
            auto const [top_point, top_point_path] = stack_.back();

            if (current_path == top_point_path)
            {
                // Backtrack and output triangles until an incorrect angle is
                // found or the stack is emptied
                while (stack_.size() >= 2u)
                {
                    auto const point_a = current_point;
                    auto const point_b_iter = std::prev(stack_.end());
                    auto const point_c_iter = std::prev(point_b_iter);
                    auto const point_b = point_b_iter->first;
                    auto const point_c = point_c_iter->first;

                    auto m = glm::mat2{};
                    m[0] = point_b - point_a;
                    m[1] = point_c - point_a;
                    const auto det = glm::determinant(m);

                    if (current_path == Path::bottom)
                    {
                        if (det < 0.0f)
                        {
                            // B C
                            // A
                            *result++ = point_a;
                            *result++ = point_c;
                            *result++ = point_b;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (det > 0.0f)
                        {
                            // A
                            // B C
                            *result++ = point_b;
                            *result++ = point_c;
                            *result++ = point_a;
                        } else {
                            break;
                        }
                    }

                    stack_.pop_back();
                }
            }
            else
            {
                // Empty the whole stack and output triangles
                for (auto const i :
                     std::views::iota(std::size_t{ 0 }, stack_.size() - 1u))
                {
                    if (current_path == Path::bottom)
                    {
                        *result++ = stack_[i].first;
                        *result++ = current_point;
                        *result++ = stack_[i + 1].first;
                    }
                    else
                    {
                        *result++ = current_point;
                        *result++ = stack_[i].first;
                        *result++ = stack_[i + 1].first;
                    }
                }

                stack_.erase(stack_.begin(), std::prev(stack_.end()));
            }

            stack_.emplace_back(current_point, current_path);
        }

        return result;
    }

    void reset()
    {
        top_path_.clear();
        bottom_path_.clear();
        stack_.clear();
    }

private:
    enum class Path : std::uint8_t
    {
        top,
        bottom,
    };

    std::deque<glm::vec2> top_path_;
    std::deque<glm::vec2> bottom_path_;
    std::vector<std::pair<glm::vec2, Path>> stack_;

    [[nodiscard]] auto paths_exhausted() const noexcept -> bool
    {
        return top_path_.empty() and bottom_path_.empty();
    }

    [[nodiscard]] auto next_point() -> std::pair<glm::vec2, Path>
    {
        if (bottom_path_.empty() or
            (not top_path_.empty() and
             top_path_.front().x < bottom_path_.front().x))
        {
            auto const point = top_path_.front();
            top_path_.pop_front();
            return { point, Path::top };
        }

        auto const point = bottom_path_.front();
        bottom_path_.pop_front();
        return { point, Path::bottom };
    }
};

} // namespace pa093::algorithm::triangulation
