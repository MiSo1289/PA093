#include <pa093/app.hpp>

#include <spdlog/spdlog.h>

#include <pa093/algorithm/convex_hull_gift_wrapping.hpp>

namespace pa093 {

App::App(glpp::glfw::Window& window)
{
    event_connections_.emplace_back(
        window.on_cursor_pos([this](glpp::glfw::CursorPosEvent const event) {
            cursor_pos_ = to_gl_coords({
                static_cast<float>(event.xpos),
                static_cast<float>(event.ypos),
            });
        }));

    event_connections_.emplace_back(window.on_framebuffer_size(
        [this](glpp::glfw::FrameBufferSizeEvent const event) {
            framebuffer_size_ = {
                static_cast<float>(event.width),
                static_cast<float>(event.height),
            };
        }));

    event_connections_.emplace_back(window.on_mouse_button(
        [this](glpp::glfw::MouseButtonEvent const event) {
            if (gui_hovered_) {
                return;
            }

            if (event.button == glpp::glfw::MouseButton::left) {
                if (event.action == glpp::glfw::KeyAction::release) {
                    spdlog::info("Adding point at {0}, {1}",
                                 cursor_pos_.x,
                                 cursor_pos_.y);

                    points_.push_back(cursor_pos_);
                    scene_dirty_ = true;
                }
            } else if (event.button == glpp::glfw::MouseButton::left) {
                
            }
        }));
}

void
App::update()
{
    if (not std::exchange(scene_dirty_, false)) {
        return;
    }

    spdlog::info("Updating scene");

    highlighted_points_.clear();

    switch (mode_) {
        case Mode::none:
            break;
        case Mode::convex_hull_gift_wrapping:
            algorithm::convex_hull_gift_wrapping(
                points_.begin(),
                points_.end(),
                std::back_inserter(highlighted_points_));
            break;
    }

    point_drawer_.set_points(points_);
    highlighted_point_drawer_.set_points(highlighted_points_);
}

void
App::draw_gui()
{
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Left-click to place points");
    ImGui::Text("Right-click to delete points");
    ImGui::NewLine();

    ImGui::Text("Show:");
    auto mode_value = static_cast<int>(mode_);
    ImGui::RadioButton("Nothing", &mode_value, static_cast<int>(Mode::none));
    ImGui::RadioButton("Convex hull (gift wrapping)",
                       &mode_value,
                       static_cast<int>(Mode::convex_hull_gift_wrapping));
    auto const new_mode = static_cast<Mode>(mode_value);
    if (std::exchange(mode_, new_mode) != new_mode) {
        scene_dirty_ = true;
    }

    ImGui::End();

    gui_hovered_ = ImGui::IsAnyItemHovered() or ImGui::IsAnyWindowHovered();
}

void
App::draw_scene()
{
    point_drawer_.set_point_size(5.0f);
    point_drawer_.draw();

    highlighted_point_drawer_.set_point_size(10.0f);
    highlighted_point_drawer_.set_color({ 1.0f, 0.0f, 0.0f, 1.0f });
    highlighted_point_drawer_.draw();
}

glm::vec2
App::to_gl_coords(glm::vec2 const screen_coords) const
{
    return glm::vec2{ -1.0f, 1.0f } + 2.0f * glm::vec2{
        screen_coords.x / framebuffer_size_.x,
        -screen_coords.y / framebuffer_size_.y,
    };
}

} // namespace pa093
