#pragma once

#include <fstream>
#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glpp/buffer.hpp>
#include <glpp/config/config.hpp>
#include <glpp/draw.hpp>
#include <glpp/shader.hpp>
#include <glpp/shader_program.hpp>
#include <glpp/uniform.hpp>
#include <glpp/vertex_array.hpp>

namespace pa093 {

class PointDrawer
{
public:
    PointDrawer()
        : program_{ [] {
            auto shader_config_file = std::ifstream{ program_path };
            auto shader_config = nlohmann::json();
            shader_config_file >> shader_config;
            return glpp::config::make_shader_program(shader_config);
        }() }
        , color_uniform_{ program_.uniform_location("color").value() }
    {
        vertex_array_.bind_attribute_buffer(
            pos_gl_buffer_.view(),
            program_.attribute_location("pos").value(),
            2u);
    }

    void set_points(std::span<glm::vec2 const> const points)
    {
        pos_gl_buffer_.buffer_data(
            { glm::value_ptr(points[0]), points.size() * 2u });
        num_points_ = points.size();
    }

    void set_color(glm::vec4 const color)
    {
        color_ = color;
    }

    void draw()
    {
        auto const program_bind = glpp::ScopedBind{program_};
        color_uniform_.load(color_);

        auto const vao_bind = glpp::ScopedBind{vertex_array_};
        glpp::draw_points(
            static_cast<glpp::Size>(num_points_), 0u, point_size_);
    }

    void set_point_size(float const point_size) noexcept
    {
        point_size_ = point_size;
    }

private:
    static constexpr auto program_path = "data/shaders/program.json";

    glpp::ShaderProgram program_;
    float point_size_ = 1.0f;
    glm::vec4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
    std::size_t num_points_ = 0u;
    glpp::Uniform<glm::vec4> color_uniform_;
    glpp::VertexArray vertex_array_;
    glpp::DynamicAttribBuffer<float> pos_gl_buffer_;
};

} // namespace pa093
