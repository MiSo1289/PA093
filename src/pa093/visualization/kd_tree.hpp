#pragma once

#include <limits>
#include <vector>

#include <glm/glm.hpp>

#include <pa093/datastructure/kd_tree.hpp>
#include <pa093/render/mesh.hpp>
#include <pa093/render/shader_cache.hpp>

namespace pa093::visualization
{

class KDTree
{
public:
    explicit KDTree(render::ShaderCache& shader_cache)
        : horizontal_lines_mesh_{ shader_cache }
        , vertical_lines_mesh_{ shader_cache }
    {
    }

    void set_tree(datastructure::KDTree2f const& tree)
    {
        horizontal_line_points_.clear();
        vertical_line_points_.clear();

        // Find bounds
        constexpr auto inf = std::numeric_limits<float>::infinity();

        auto min = glm::vec2{ inf, inf };
        auto max = glm::vec2{ -inf, -inf };

        for (auto const point : tree.points())
        {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        // Build meshes
        visit_subtree(tree, tree.root(), 0u, min, max);

        horizontal_lines_mesh_.set_vertex_positions(horizontal_line_points_);
        vertical_lines_mesh_.set_vertex_positions(vertical_line_points_);
    }

    void draw(glm::vec4 horizontal_color, glm::vec4 vertical_color)
    {
        horizontal_lines_mesh_.draw(glpp::DrawPrimitive::lines,
                                    horizontal_color);
        vertical_lines_mesh_.draw(glpp::DrawPrimitive::lines, vertical_color);
    }

private:
    render::DynamicMesh2d horizontal_lines_mesh_;
    render::DynamicMesh2d vertical_lines_mesh_;
    std::vector<glm::vec2> horizontal_line_points_;
    std::vector<glm::vec2> vertical_line_points_;

    void visit_subtree(datastructure::KDTree2f const& tree,
                       datastructure::KDTree2f::node_id_type const node_id,
                       std::size_t const depth,
                       glm::vec2 const min,
                       glm::vec2 const max)
    {
        if (not node_id or tree.is_leaf(node_id))
        {
            return;
        }

        auto const& node = tree.node(node_id);
        auto const current_dim = static_cast<int>(depth % 2u);

        // Create the dividing line
        auto line_start = min;
        line_start[current_dim] = node.pivot;
        auto line_end = max;
        line_end[current_dim] = node.pivot;

        if (current_dim == 0)
        {
            vertical_line_points_.push_back(line_start);
            vertical_line_points_.push_back(line_end);
        }
        else
        {
            horizontal_line_points_.push_back(line_start);
            horizontal_line_points_.push_back(line_end);
        }

        // Descend to children
        visit_subtree(tree, node.left, depth + 1u, min, line_end);
        visit_subtree(tree, node.right, depth + 1u, line_start, max);
    }
};

} // namespace pa093::visualization
