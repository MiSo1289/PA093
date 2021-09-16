#pragma once

#include <limits>
#include <optional>
#include <random>
#include <vector>

#include <boost/signals2.hpp>
#include <glm/glm.hpp>
#include <glpp/glfw/window.hpp>
#include <imgui.h>

#include <pa093/algorithm/gift_wrapping_convex_hull.hpp>
#include <pa093/render/mesh.hpp>
#include <pa093/render/shader_cache.hpp>

namespace pa093
{

class App
{
public:
    static constexpr auto window_title = "PA093";
    static constexpr auto init_window_mode = glpp::glfw::WindowMode{
        .window_type = glpp::glfw::WindowType::windowed,
        // This resolution was prescribed by God
        .width = 640,
        .height = 480,
    };

    explicit App(glpp::glfw::Window& window);

    void update();

    void draw_gui();

    void draw_scene();

private:
    enum class Mode : int
    {
        none,
        gift_wrapping_convex_hull,
    };

    static constexpr auto point_highlight_radius = 0.05f;
    static constexpr auto default_color = glm::vec4(1.0f);
    static constexpr auto highlighted_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    static constexpr auto convex_hull_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // Algorithms
    algorithm::GiftWrappingConvexHull gift_wrapping_convex_hull_;

    // Render components
    render::ShaderCache shader_cache_;
    render::DynamicMesh point_mesh_{ shader_cache_ };
    render::DynamicMesh highlighted_point_mesh_{ shader_cache_ };
    render::DynamicMesh convex_hull_point_mesh_{ shader_cache_ };

    // State
    std::mt19937_64 rng_;
    bool scene_dirty_ = false;
    bool gui_hovered_ = false;
    int num_points_to_generate_ = 10;
    Mode mode_ = Mode::none;
    glm::vec2 framebuffer_size_ = {
        init_window_mode.width,
        init_window_mode.height,
    };
    glm::vec2 cursor_pos_ = {};
    std::optional<std::size_t> highlighted_point_ = std::nullopt;
    std::optional<std::size_t> dragged_point_ = std::nullopt;
    std::vector<glm::vec2> points_ = {};
    std::vector<glm::vec2> convex_hull_points_ = {};

    // Events
    std::vector<boost::signals2::scoped_connection> event_connections_;

    [[nodiscard]] auto point_from_screen_coords(glm::vec2 screen_coords) const
        -> glm::vec2;

    void set_mode(Mode mode);

    void add_point(glm::vec2 pos);

    void remove_point(std::size_t point_index);

    void remove_all_points();

    void generate_random_points(std::size_t count);

    [[nodiscard]] auto find_closest_point(
        glm::vec2 pos,
        float max_search_radius = std::numeric_limits<float>::infinity()) const
        -> std::optional<std::size_t>;


};

} // namespace pa093
