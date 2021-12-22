#pragma once

#include <limits>
#include <optional>
#include <random>
#include <vector>

#include <boost/signals2.hpp>
#include <glm/glm.hpp>
#include <glpp/glfw/window.hpp>
#include <imgui.h>

#include <pa093/algorithm/convex_hull/gift_wrapping.hpp>
#include <pa093/algorithm/convex_hull/graham_scan.hpp>
#include <pa093/algorithm/kd_tree/build_kd_tree.hpp>
#include <pa093/algorithm/triangulation/delaunay.hpp>
#include <pa093/algorithm/triangulation/dual_graph.hpp>
#include <pa093/algorithm/triangulation/sweep_line.hpp>
#include <pa093/datastructure/kd_tree.hpp>
#include <pa093/render/mesh.hpp>
#include <pa093/render/shader_cache.hpp>
#include <pa093/visualization/kd_tree.hpp>

namespace pa093
{

class App
{
public:
    static constexpr auto window_title = "PA093";
    static constexpr auto init_window_mode = glpp::glfw::WindowMode{
        .window_type = glpp::glfw::WindowType::windowed,
        // The holy resolution
        .width = 640,
        .height = 480,
    };

    explicit App(glpp::glfw::Window& window);

    void update();

    void draw_gui();

    void draw_scene();

private:
    enum class PolygonMode : int
    {
        none = 0,
        all_points,
        gift_wrapping_convex_hull,
        graham_scan_convex_hull,
    };

    enum class TriangulationMode : int
    {
        none = 0,
        sweep_line,
        delaunay,
        delaunay_plus_voronoi,
    };

    enum class PartitioningMode : int
    {
        none = 0,
        kd_tree,
    };

    static constexpr auto font_size_pixels_unscaled = 13.0f;
    static constexpr auto min_toolbar_width_pixels = 300.0f;
    static constexpr auto default_color = glm::vec4(1.0f);
    static constexpr auto highlighted_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    static constexpr auto polygon_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    static constexpr auto triangle_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    static constexpr auto voronoi_color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    static constexpr auto kd_tree_vertical_color =
        glm::vec4(0.3f, 1.0f, 0.3f, 1.0f);
    static constexpr auto kd_tree_horizontal_color =
        glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    static constexpr auto point_highlight_radius = 0.05f;
    static constexpr auto max_generated_points = 1'000;
    static constexpr auto voronoi_hull_edge_length = 3.0f;

    // Algorithms
    algorithm::convex_hull::GiftWrapping gift_wrapping_;
    algorithm::convex_hull::GrahamScan graham_scan_;
    algorithm::kd_tree::BuildKDTree2f build_kd_tree_;
    algorithm::triangulation::SweepLine sweep_line_;
    algorithm::triangulation::Delaunay delaunay_;
    algorithm::triangulation::DualGraph voronoi_{ voronoi_hull_edge_length };

    // Datastructures
    datastructure::KDTree2f kd_tree_;

    // Render components
    render::ShaderCache shader_cache_;
    render::DynamicMesh2d point_mesh_{ shader_cache_ };
    render::DynamicMesh2d highlighted_point_mesh_{ shader_cache_ };
    render::DynamicMesh2d polygon_mesh_{ shader_cache_ };
    render::DynamicMesh2d triangle_mesh_{ shader_cache_ };
    render::DynamicMesh2d voronoi_mesh_{ shader_cache_ };
    visualization::KDTree kd_tree_visualization_{ shader_cache_ };

    // State
    std::mt19937_64 rng_;
    bool scene_dirty_ = false;
    bool gui_hovered_ = false;
    int num_points_to_generate_ = 10;
    PolygonMode polygon_mode_ = PolygonMode::none;
    TriangulationMode triangulation_mode_ = TriangulationMode::none;
    PartitioningMode partitioning_mode_ = PartitioningMode::none;
    glm::vec2 framebuffer_size_ = {
        init_window_mode.width,
        init_window_mode.height,
    };
    glm::vec2 cursor_pos_ = {};
    std::optional<std::size_t> highlighted_point_ = std::nullopt;
    std::optional<std::size_t> dragged_point_ = std::nullopt;
    std::vector<glm::vec2> points_ = {};
    std::vector<glm::vec2> polygon_points_ = {};
    std::vector<glm::vec2> triangle_points_ = {};
    std::vector<glm::vec2> voronoi_points_ = {};

    // Events
    std::vector<boost::signals2::scoped_connection> event_connections_;

    void set_content_scale(glm::vec2 scale);

    void set_polygon_mode(PolygonMode mode);

    void set_triangulation_mode(TriangulationMode mode);

    void set_partitioning_mode(PartitioningMode mode);

    void add_point(glm::vec2 pos);

    void remove_point(std::size_t point_index);

    void remove_all_points();

    void generate_random_points(std::size_t count);

    [[nodiscard]] auto point_from_screen_coords(glm::vec2 screen_coords) const
        -> glm::vec2;

    [[nodiscard]] auto find_closest_point(
        glm::vec2 pos,
        float max_search_radius = std::numeric_limits<float>::infinity()) const
        -> std::optional<std::size_t>;
};

} // namespace pa093
