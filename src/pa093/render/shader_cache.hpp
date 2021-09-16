#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <glpp/shader_program.hpp>

namespace pa093::render
{

[[nodiscard]] auto load_shader_from_config(
    std::filesystem::path const& shader_config_path) -> glpp::ShaderProgram;

class ShaderCache
{
public:
    [[nodiscard]] auto operator[](
        std::filesystem::path const& shader_config_path)
        -> glpp::ShaderProgram const&;

private:
    std::unordered_map<std::string, glpp::ShaderProgram> programs_;
};

} // namespace pa093::render
