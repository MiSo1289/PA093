#pragma once

#include <vector>

#include <boost/signals2.hpp>
#include <glm/glm.hpp>
#include <glpp/glfw/window.hpp>
#include <imgui.h>

#include <pa093/point_drawer.hpp>

namespace pa093 {

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
        convex_hull_gift_wrapping,
    };

    static constexpr auto remove_point_search_radius_ = 0.05f;

    // Components
    PointDrawer point_drawer_;
    PointDrawer highlighted_point_drawer_;

    // State
    glm::vec2 framebuffer_size_ = {
        init_window_mode.width,
        init_window_mode.height,
    };
    glm::vec2 cursor_pos_ = {};
    bool scene_dirty_ = false;
    bool gui_hovered_ = false;
    Mode mode_ = Mode::none;
    std::vector<glm::vec2> points_ = {};
    std::vector<glm::vec2> highlighted_points_ = {};

    // Events
    std::vector<boost::signals2::scoped_connection> event_connections_;

    [[nodiscard]] glm::vec2 to_gl_coords(glm::vec2 screen_coords) const;
};

} // namespace pa093
