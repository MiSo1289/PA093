#include <pa093/render/mesh.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace pa093::render
{

DynamicMesh::DynamicMesh(ShaderCache& shader_cache)
    : program_{ shader_cache[program_config_path] }
    , color_uniform_{ program_.uniform_location("color").value() }
{
    vertex_array_.bind_attribute_buffer(
        pos_gl_buffer_.view(), program_.attribute_location("pos").value(), 2u);
}

void
DynamicMesh::set_vertex_positions(std::span<glm::vec2 const> const points)
{
    pos_gl_buffer_.buffer_data(
        { glm::value_ptr(points[0]), points.size() * 2u });
    num_points_ = points.size();
}

void
DynamicMesh::draw(glpp::DrawPrimitive const primitive, glm::vec4 const color)
{
    auto const program_bind = glpp::ScopedBind{ program_ };
    color_uniform_.load(color);

    auto const vao_bind = glpp::ScopedBind{ vertex_array_ };
    glpp::draw(primitive, static_cast<glpp::Size>(num_points_));
}

void
DynamicMesh::draw_points(float const point_size, glm::vec4 const color)
{
    auto const program_bind = glpp::ScopedBind{ program_ };
    color_uniform_.load(color);

    auto const vao_bind = glpp::ScopedBind{ vertex_array_ };
    glpp::draw_points(static_cast<glpp::Size>(num_points_), 0, point_size);
}

} // namespace pa093
