#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glpp/buffer.hpp>
#include <glpp/draw.hpp>
#include <glpp/shader_program.hpp>
#include <glpp/uniform.hpp>
#include <glpp/vertex_array.hpp>

#include <pa093/render/shader_cache.hpp>

namespace pa093::render
{

class DynamicMesh2d
{
public:
    explicit DynamicMesh2d(ShaderCache& shader_cache);

    void set_vertex_positions(std::span<glm::vec2 const> points);

    void draw(glpp::DrawPrimitive primitive, glm::vec4 color = glm::vec4(1.0f));

    void draw_points(float point_size = 1.0f,
                     glm::vec4 color = glm::vec4(1.0f));

private:
    static constexpr auto program_config_path = "data/shader/program.json";

    glpp::ShaderProgram const& program_;
    glpp::Uniform<glm::vec4> color_uniform_;
    glpp::VertexArray vertex_array_;
    glpp::DynamicAttribBuffer<float> pos_gl_buffer_;
    std::size_t num_points_ = 0u;
};

} // namespace pa093::render
