#include <pa093/app.hpp>

#include <glm/gtx/norm.hpp>
#include <spdlog/spdlog.h>

#include <pa093/algorithm/gift_wrapping_convex_hull_2d.hpp>

namespace pa093
{

App::App(glpp::glfw::Window& window)
{
    auto rd = std::random_device{};
    rng_.seed(rd());

    auto const [hor_scale, vert_scale] = window.content_scale();
    set_content_scale({ hor_scale, vert_scale });

    event_connections_.emplace_back(window.on_content_scale(
        [this](glpp::glfw::ContentScaleEvent const event) {
            set_content_scale({ event.hor_scale, event.vert_scale });
        }));

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
                    if (highlighted_point_)
                    {
                        dragged_point_ = *highlighted_point_;
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
                    if (highlighted_point_)
                    {
                        remove_point(*highlighted_point_);
                        highlighted_point_.reset();
                        dragged_point_.reset();
                    }
                }
            }
        }));
}

void
App::update()
{
    if (dragged_point_)
    {
        // Update dragged point
        points_[*dragged_point_] = cursor_pos_;
        highlighted_point_ = *dragged_point_;
        scene_dirty_ = true;
    }
    else if (gui_hovered_)
    {
        highlighted_point_.reset();
    }
    else if (auto const new_highlighted_point =
                 find_closest_point(cursor_pos_, point_highlight_radius);
             new_highlighted_point != highlighted_point_)
    {
        // Update hovered point
        highlighted_point_ = new_highlighted_point;
    }

    // Show / hide highlighted point
    if (highlighted_point_)
    {
        highlighted_point_mesh_.set_vertex_positions(
            std::span{ &points_[*highlighted_point_], 1u });
    }
    else
    {
        highlighted_point_mesh_.set_vertex_positions({});
    }

    if (std::exchange(scene_dirty_, false))
    {
        // Update scene geometry
        convex_hull_points_.clear();

        switch (mode_)
        {
            case Mode::none:
                break;
            case Mode::gift_wrapping_convex_hull:
                gift_wrapping_convex_hull_2d_(
                    points_, std::back_inserter(convex_hull_points_));
                break;
        }

        point_mesh_.set_vertex_positions(points_);
        convex_hull_point_mesh_.set_vertex_positions(convex_hull_points_);
    }
}

void
App::draw_gui()
{
    if (not ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    ImGui::Dummy({ min_toolbar_width_pixels, 0 });

    if (ImGui::CollapsingHeader("Show", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto mode_value = static_cast<int>(mode_);
        ImGui::RadioButton(
            "Nothing", &mode_value, static_cast<int>(Mode::none));
        ImGui::RadioButton("Convex hull (gift wrapping)",
                           &mode_value,
                           static_cast<int>(Mode::gift_wrapping_convex_hull));
        set_mode(static_cast<Mode>(mode_value));

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Points", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Generate"))
        {
            generate_random_points(
                static_cast<std::size_t>(num_points_to_generate_));
        }
        ImGui::SameLine();
        ImGui::SliderInt("", &num_points_to_generate_, 1, max_generated_points);
        num_points_to_generate_ =
            std::clamp(num_points_to_generate_, 1, max_generated_points);

        if (ImGui::Button("Clear"))
        {
            remove_all_points();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Left-click to add points");
        ImGui::Text("Left-click + drag to move points");
        ImGui::Text("Right-click to remove points");

        ImGui::Spacing();
        ImGui::Separator();
        if (highlighted_point_)
        {
            ImGui::Text("%f, %f (%zu)",
                        cursor_pos_.x,
                        cursor_pos_.y,
                        *highlighted_point_);
        }
        else
        {
            ImGui::Text("%f, %f", cursor_pos_.x, cursor_pos_.y);
        }

        ImGui::Text("%zu points", points_.size());

        if (mode_ == Mode::gift_wrapping_convex_hull)
        {
            ImGui::Text("%zu hull points", convex_hull_points_.size());
        }

        ImGui::Spacing();
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
            convex_hull_point_mesh_.draw(glpp::DrawPrimitive::line_loop,
                                         convex_hull_color);
            convex_hull_point_mesh_.draw_points(10.0f, convex_hull_color);
            break;
    }

    highlighted_point_mesh_.draw_points(10.0f, highlighted_color);
}

void
App::set_content_scale(glm::vec2 const scale)
{
    content_scale_ = scale;

    ImGui::GetIO().Fonts->ClearFonts();

    auto cfg = ImFontConfig{};
    cfg.SizePixels = font_size_pixels_unscaled * std::max(scale.x, scale.y);
    ImGui::GetIO().Fonts->AddFontDefault(&cfg);
}

void
App::set_mode(Mode const mode)
{
    if (std::exchange(mode_, mode) != mode)
    {
        scene_dirty_ = true;
    }
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

void
App::remove_all_points()
{
    spdlog::info("Removing all points");

    points_.clear();
    scene_dirty_ = true;
}

void
App::generate_random_points(std::size_t const count)
{
    spdlog::info("Generating {0} points", count);

    auto coord_dist = std::uniform_real_distribution{ -1.0f, 1.0f };

    std::generate_n(std::back_inserter(points_),
                    count,
                    [&] {
                        return glm::vec2{ coord_dist(rng_), coord_dist(rng_) };
                    });
    scene_dirty_ = true;
}

auto
App::point_from_screen_coords(glm::vec2 screen_coords) const -> glm::vec2
{
    return glm::vec2{ -1.0f, 1.0f } + 2.0f * glm::vec2{
        screen_coords.x / framebuffer_size_.x,
        -screen_coords.y / framebuffer_size_.y,
    };
}

auto
App::find_closest_point(glm::vec2 const pos,
                        float const max_search_radius) const
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
