#include <pa093/app.hpp>

#include <glm/gtx/norm.hpp>
#include <spdlog/spdlog.h>

#include <pa093/algorithm/gift_wrapping_convex_hull.hpp>

namespace pa093
{

App::App(glpp::glfw::Window& window)
{
    event_connections_.emplace_back(window.on_cursor_pos(
        [this](glpp::glfw::CursorPosEvent const event)
        {
            cursor_pos_ = point_from_screen_coords({
                static_cast<float>(event.xpos),
                static_cast<float>(event.ypos),
            });
        }));

    event_connections_.emplace_back(window.on_framebuffer_size(
        [this](glpp::glfw::FrameBufferSizeEvent const event)
        {
            framebuffer_size_ = {
                static_cast<float>(event.width),
                static_cast<float>(event.height),
            };
        }));

    event_connections_.emplace_back(window.on_mouse_button(
        [this](glpp::glfw::MouseButtonEvent const event)
        {
            if (gui_hovered_)
            {
                return;
            }

            if (event.button == glpp::glfw::MouseButton::left)
            {
                if (event.action == glpp::glfw::KeyAction::press)
                {
                    if (auto const closest_point = find_closest_point(
                            cursor_pos_, drag_point_search_radius))
                    {
                        dragged_point_ = *closest_point;
                    }
                    else
                    {
                        add_point(cursor_pos_);
                    }
                }
                else if (event.action == glpp::glfw::KeyAction::release)
                {
                    dragged_point_.reset();
                }
            }
            else if (event.button == glpp::glfw::MouseButton::right)
            {
                if (event.action == glpp::glfw::KeyAction::press)
                {
                    if (auto const closest_point = find_closest_point(
                            cursor_pos_, remove_point_search_radius))
                    {
                        remove_point(*closest_point);
                    }
                }
            }
        }));
}

void
App::update()
{
    if (dragged_point_.has_value())
    {
        points_[*dragged_point_] = cursor_pos_;
        scene_dirty_ = true;
    }

    if (not std::exchange(scene_dirty_, false))
    {
        return;
    }

    highlighted_points_.clear();

    switch (mode_)
    {
        case Mode::none:
            break;
        case Mode::gift_wrapping_convex_hull:
            gift_wrapping_convex_hull_(points_.begin(),
                                       points_.end(),
                                       std::back_inserter(highlighted_points_));
            break;
    }

    point_mesh_.set_vertex_positions(points_);
    highlighted_point_mesh_.set_vertex_positions(highlighted_points_);
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
                       static_cast<int>(Mode::gift_wrapping_convex_hull));
    auto const new_mode = static_cast<Mode>(mode_value);
    if (std::exchange(mode_, new_mode) != new_mode)
    {
        scene_dirty_ = true;
    }

    ImGui::End();

    gui_hovered_ = ImGui::IsAnyItemHovered() or ImGui::IsAnyWindowHovered();
}

void
App::draw_scene()
{
    point_mesh_.draw_points(5.0f, default_color);

    switch (mode_)
    {
        case Mode::none:
            break;
        case Mode::gift_wrapping_convex_hull:
            highlighted_point_mesh_.draw(glpp::DrawPrimitive::line_loop,
                                         highlighted_color);
            highlighted_point_mesh_.draw_points(10.0f, highlighted_color);
            break;
    }
}

auto
App::point_from_screen_coords(glm::vec2 screen_coords) const -> glm::vec2
{
    return glm::vec2{ -1.0f, 1.0f } + 2.0f * glm::vec2{
        screen_coords.x / framebuffer_size_.x,
        -screen_coords.y / framebuffer_size_.y,
    };
}

void
App::add_point(glm::vec2 const pos)
{
    spdlog::info("Adding point at {0}, {1}", pos.x, pos.y);

    points_.push_back(pos);
    scene_dirty_ = true;
}

void
App::remove_point(std::size_t const point_index)
{
    auto const point_iter = points_.begin() + point_index;
    spdlog::info("Removing point at {0}, {1}", point_iter->x, point_iter->y);

    points_.erase(point_iter);
    scene_dirty_ = true;
}

auto
App::find_closest_point(glm::vec2 const pos, float const max_search_radius)
    -> std::optional<std::size_t>
{
    auto const max_rad2 = max_search_radius * max_search_radius;

    if (auto const match = std::ranges::min_element(
            points_,
            std::less{},
            [&](auto const point) { return glm::length2(point - pos); });
        match != points_.end() and glm::length2(*match - pos) <= max_rad2)
    {
        return std::distance(points_.begin(), match);
    }

    return std::nullopt;
}

} // namespace pa093
