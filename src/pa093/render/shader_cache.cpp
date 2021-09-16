#include <pa093/render/shader_cache.hpp>

#include <fstream>

#include <glpp/config/config.hpp>
#include <nlohmann/json.hpp>

namespace pa093::render
{

auto
load_shader_from_config(std::filesystem::path const& shader_config_path)
    -> glpp::ShaderProgram
{
    auto shader_config_file = std::ifstream{ shader_config_path };
    auto shader_config = nlohmann::json();
    shader_config_file >> shader_config;
    return glpp::config::make_shader_program(shader_config);
}

auto
ShaderCache::operator[](std::filesystem::path const& shader_config_path)
    -> glpp::ShaderProgram const&
{
    auto cannonical_path =
        std::filesystem::canonical(shader_config_path).string();
    auto match = programs_.find(cannonical_path);

    if (match == programs_.end())
    {
        std::tie(match, std::ignore) =
            programs_.emplace(std::move(cannonical_path),
                              load_shader_from_config(shader_config_path));
    }

    return match->second;
}

} // namespace pa093::render
