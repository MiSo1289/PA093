#pragma once

#include <vector>

#include <pa093/datastructure/kd_tree.hpp>

namespace pa093::algorithm::kd_tree
{

template<typename T, std::size_t dim>
class BuildKDTree
{
public:
    using tree_type = datastructure::KDTree<T, dim>;
    using node_id_type = typename tree_type::node_id_type;
    using node_type = typename tree_type::node_type;
    using point_type = typename tree_type::point_type;

    template<std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, point_type>
    void operator()(R&& range, tree_type& tree)
    {
        return (*this)(
            std::ranges::begin(range), std::ranges::end(range), tree);
    }

    template<std::input_iterator I, std::sentinel_for<I> S>
    requires std::same_as<std::iter_value_t<I>, point_type>
    void operator()(I const first, S const last, tree_type& tree)
    {
        reset();
        tree.clear();

        std::ranges::copy(first, last, std::back_inserter(points_));

        build_subtree(tree, points_.begin(), points_.end(), 0u);
    }

    void reset() { points_.clear(); }

private:
    std::vector<point_type> points_;

    template<std::forward_iterator I, std::sentinel_for<I> S>
    requires std::same_as<std::iter_value_t<I>, point_type>
    auto build_subtree(tree_type& tree,
                       I const first,
                       S const last,
                       std::size_t const depth) -> node_id_type
    {
        auto const n = std::distance(first, last);

        // Cases for bottom of recursion
        if (n == 0u)
        {
            return node_type::null;
        }
        if (n == 1u)
        {
            return tree.add_leaf(*first);
        }

        // Create a new node
        auto const parent = tree.add_node();

        // Find pivot and partition the input
        auto const middle = std::next(first, n / 2u);
        auto const current_dim = static_cast<int>(depth % dim);

        std::ranges::nth_element(first,
                                 middle,
                                 last,
                                 std::less{},
                                 [=](point_type const point)
                                 { return point[current_dim]; });

        auto const pivot = (*middle)[current_dim];

        // Recurse to both halves
        auto const left = build_subtree(tree, first, middle, depth + 1u);
        auto const right = build_subtree(tree, middle, last, depth + 1u);

        auto& parent_node = tree.node(parent);
        parent_node.pivot = pivot;
        parent_node.left = left;
        parent_node.right = right;

        return parent;
    }
};

using BuildKDTree2f = BuildKDTree<float, 2u>;

} // namespace pa093::algorithm::kd_tree
