#include <pa093/app.hpp>

#include <glm/gtx/norm.hpp>
#include <spdlog/spdlog.h>

#include "pa093/algorithm/convex_hull/gift_wrapping.hpp"

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
        polygon_points_.clear();
        triangle_points_.clear();
        kd_tree_.clear();

        switch (polygon_mode_)
        {
            case PolygonMode::none:
                break;
            case PolygonMode::all_points:
                polygon_points_ = points_;
                break;
            case PolygonMode::gift_wrapping_convex_hull:
                gift_wrapping_(points_, std::back_inserter(polygon_points_));
                break;
            case PolygonMode::graham_scan_convex_hull:
                graham_scan_(points_, std::back_inserter(polygon_points_));
                break;
        }

        switch (triangulation_mode_)
        {
            case TriangulationMode::none:
                break;
            case TriangulationMode::sweep_line:
                sweep_line_(polygon_points_,
                            std::back_inserter(triangle_points_));
                break;
        }

        switch (partitioning_mode_)
        {
            case PartitioningMode::none:
                break;
            case PartitioningMode::kd_tree:
                build_kd_tree_(points_, kd_tree_);
                break;
        }

        point_mesh_.set_vertex_positions(points_);
        polygon_mesh_.set_vertex_positions(polygon_points_);
        triangle_mesh_.set_vertex_positions(triangle_points_);
        kd_tree_visualization_.set_tree(kd_tree_);
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

    if (ImGui::CollapsingHeader("Polygon", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("polygon");

        auto mode_value = static_cast<int>(polygon_mode_);
        ImGui::RadioButton(
            "None", &mode_value, static_cast<int>(PolygonMode::none));
        ImGui::RadioButton("All points",
                           &mode_value,
                           static_cast<int>(PolygonMode::all_points));
        ImGui::RadioButton(
            "Convex hull (gift wrapping)",
            &mode_value,
            static_cast<int>(PolygonMode::gift_wrapping_convex_hull));
        ImGui::RadioButton(
            "Convex hull (Graham's scan)",
            &mode_value,
            static_cast<int>(PolygonMode::graham_scan_convex_hull));
        set_polygon_mode(static_cast<PolygonMode>(mode_value));

        ImGui::Spacing();
        ImGui::Separator();

        if (std::ranges::count(
                std::array{
                    PolygonMode::gift_wrapping_convex_hull,
                    PolygonMode::graham_scan_convex_hull,
                },
                polygon_mode_))
        {
            ImGui::Text("%zu hull points", polygon_points_.size());
        }

        ImGui::Spacing();

        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Triangulation",
                                ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("triangulation");

        auto mode_value = static_cast<int>(triangulation_mode_);
        ImGui::RadioButton(
            "None", &mode_value, static_cast<int>(TriangulationMode::none));
        ImGui::RadioButton("Sweep line",
                           &mode_value,
                           static_cast<int>(TriangulationMode::sweep_line));
        set_triangulation_mode(static_cast<TriangulationMode>(mode_value));

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Spacing();

        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Partitioning", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("partitioning");

        auto mode_value = static_cast<int>(partitioning_mode_);
        ImGui::RadioButton(
            "None", &mode_value, static_cast<int>(PartitioningMode::none));
        ImGui::RadioButton("k-D tree",
                           &mode_value,
                           static_cast<int>(PartitioningMode::kd_tree));
        set_partitioning_mode(static_cast<PartitioningMode>(mode_value));

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Spacing();

        ImGui::PopID();
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
                        points_[*highlighted_point_].x,
                        points_[*highlighted_point_].y,
                        *highlighted_point_);
        }
        else
        {
            ImGui::Text("%f, %f", cursor_pos_.x, cursor_pos_.y);
        }

        ImGui::Text("%zu points", points_.size());

        ImGui::Spacing();
    }

    ImGui::End();

    gui_hovered_ = ImGui::IsAnyItemHovered() or ImGui::IsAnyWindowHovered();
}

void
App::draw_scene()
{
    if (triangulation_mode_ != TriangulationMode::none)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        triangle_mesh_.draw(glpp::DrawPrimitive::triangles, triangle_color);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (partitioning_mode_ != PartitioningMode::none)
    {
        kd_tree_visualization_.draw(kd_tree_horizontal_color,
                                    kd_tree_vertical_color);
    }

    point_mesh_.draw_points(5.0f, default_color);

    if (polygon_mode_ != PolygonMode::none)
    {
        polygon_mesh_.draw(glpp::DrawPrimitive::line_loop, polygon_color);
        polygon_mesh_.draw_points(10.0f, polygon_color);
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
App::set_polygon_mode(PolygonMode const mode)
{
    if (std::exchange(polygon_mode_, mode) != mode)
    {
        scene_dirty_ = true;
    }
}

void
App::set_triangulation_mode(TriangulationMode const mode)
{
    if (std::exchange(triangulation_mode_, mode) != mode)
    {
        scene_dirty_ = true;
    }
}

void
App::set_partitioning_mode(PartitioningMode const mode)
{
    if (std::exchange(partitioning_mode_, mode) != mode)
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
