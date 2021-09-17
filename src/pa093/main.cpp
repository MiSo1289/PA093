#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <thread>

#include <glpp/draw.hpp>
#include <glpp/glfw/glfw.hpp>
#include <glpp/glfw/window.hpp>
#include <glpp/imgui/imgui.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <pa093/app.hpp>

namespace
{

inline constexpr auto min_frame_duration =
    std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<float>{ 1.0f } / 60.0f);

}

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

        ImGui::StyleColorsClassic();

        auto frame_start = std::chrono::steady_clock::now();

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

            // Check frame duration and sleep if necessary
            auto frame_end = std::chrono::steady_clock::now();
            auto const expected_frame_end = frame_start + min_frame_duration;

            if (frame_end < expected_frame_end)
            {
                std::this_thread::sleep_until(expected_frame_end);
                frame_end = expected_frame_end;
            }

            frame_start = frame_end;
        }

        return EXIT_SUCCESS;
    }
    catch (std::exception const& error)
    {
        spdlog::error("Uncaught exception in main thread: {0}", error.what());
        return EXIT_FAILURE;
    }
}
