#include <chrono>
#include <thread>

#include <glpp/draw.hpp>
#include <glpp/glfw/glfw.hpp>
#include <glpp/glfw/window.hpp>
#include <glpp/imgui/imgui.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <pa093/app.hpp>

auto
main() -> int
{
    using namespace std::literals;

    try
    {
        spdlog::set_level(spdlog::level::debug);

        auto glfw = glpp::glfw::Glfw{};
        auto window = glpp::glfw::Window{
            glfw,
            pa093::App::init_window_mode,
            pa093::App::window_title,
        };
        auto imgui = glpp::imgui::ImGui{ window };
        auto app = pa093::App{ window };

        ImGui::StyleColorsLight();

        while (not window.should_close())
        {
            glpp::clear_color({ 0.0f, 0.0f, 0.0f, 1.0f });
            window.poll_events();
            imgui.new_frame();

            app.draw_gui();
            app.update();
            app.draw_scene();

            imgui.render();
            window.swap_buffers();
            std::this_thread::sleep_for(10ms);
        }
    }
    catch (std::exception const& error)
    {
        spdlog::error("Uncaught exception in main thread: {0}", error.what());
        return 2;
    }
}
