#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <vector>

#include <glm/glm.hpp>
#include <gsl/gsl_assert>

namespace pa093::datastructure
{

template<typename T, std::size_t dim_>
class KDTree
{
public:
    using scalar_type = T;
    using point_type = glm::vec<dim_, scalar_type>;
    using node_id_type = std::uint64_t;

    static constexpr auto dim = dim_;

    struct node_type
    {
        static constexpr auto null = node_id_type{};
        static constexpr auto leaf_mask = node_id_type{ 1llu << 63llu };

        node_id_type left = null;
        node_id_type right = null;
        scalar_type pivot = {};
    };

    [[nodiscard]] static auto is_leaf(node_id_type const id) noexcept -> bool
    {
        return id & node_type::leaf_mask;
    }

    [[nodiscard]] auto node(node_id_type const id) noexcept -> node_type&
    {
        Expects(id != node_type::null and not is_leaf(id));
        return nodes_[id - 1u];
    }

    [[nodiscard]] auto node(node_id_type const id) const noexcept
        -> node_type const&
    {
        Expects(id != node_type::null and not is_leaf(id));
        return nodes_[id - 1u];
    }

    [[nodiscard]] auto leaf(node_id_type const id) const noexcept -> point_type
    {
        Expects(is_leaf(id));
        return &leaves_[id & ~node_type::leaf_mask];
    }

    [[nodiscard]] auto root() const noexcept -> node_id_type
    {
        if (not nodes_.empty())
        {
            // First node is root
            return node_id_type{ 1 };
        }
        if (not leaves_.empty())
        {
            // Root is (the only) leaf
            return node_type::leaf_mask;
        }
        // Empty tree
        return node_type::null;
    }

    void clear()
    {
        nodes_.clear();
        leaves_.clear();
    }

    auto add_node() -> node_id_type
    {
        nodes_.emplace_back();
        return node_id_type{ nodes_.size() };
    }

    auto add_leaf(point_type const point) -> node_id_type
    {
        auto const id = node_id_type{ leaves_.size() | node_type::leaf_mask };
        leaves_.push_back(point);
        return id;
    }

    [[nodiscard]] auto points() const noexcept
        -> std::ranges::random_access_range auto
    {
        return leaves_ | std::views::all;
    }

private:
    std::vector<node_type> nodes_;
    std::vector<point_type> leaves_;
};

using KDTree2f = KDTree<float, 2u>;

} // namespace pa093::datastructure
