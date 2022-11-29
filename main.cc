#include "log.h"
#include "game.h"
#include "system.h"
#include "mouseevent.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>

int main()
{
    constexpr auto Width = 800;
    constexpr auto Height = 600;

    glfwInit();
    glfwSetErrorCallback(
        [](int error, const char *description) { panic("GLFW error %08x: %s\n", error, description); });

    {
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window(
            glfwCreateWindow(Width, Height, "test", nullptr, nullptr), glfwDestroyWindow);

        glfwMakeContextCurrent(window.get());
        glfwSwapInterval(1);

        glewInit();

        log("Vendor: %s\n", glGetString(GL_VENDOR));
        log("Renderer: %s\n", glGetString(GL_RENDERER));
        log("Version: %s\n", glGetString(GL_VERSION));

        {
            System::initialize();

            auto game = std::make_unique<Game>();

            glfwSetWindowUserPointer(window.get(), game.get());
            glfwSetMouseButtonCallback(window.get(), [](GLFWwindow *window, int glfwButton, int glfwAction, int mods) {
                auto button = [glfwButton] {
                    switch (glfwButton)
                    {
                    case GLFW_MOUSE_BUTTON_LEFT:
                        return miniui::MouseButtons::Left;
                    case GLFW_MOUSE_BUTTON_RIGHT:
                        return miniui::MouseButtons::Right;
                    case GLFW_MOUSE_BUTTON_MIDDLE:
                        return miniui::MouseButtons::Middle;
                    default:
                        return miniui::MouseButtons::None;
                    }
                }();
                if (button != miniui::MouseButtons::None)
                {
                    auto *game = reinterpret_cast<Game *>(glfwGetWindowUserPointer(window));
                    switch (glfwAction)
                    {
                    case GLFW_PRESS:
                        game->onMouseButtonPress(button);
                        break;
                    case GLFW_RELEASE:
                        game->onMouseButtonRelease(button);
                        break;
                    default:
                        break;
                    }
                }
            });
            glfwSetCursorPosCallback(window.get(), [](GLFWwindow *window, double x, double y) {
                auto *game = reinterpret_cast<Game *>(glfwGetWindowUserPointer(window));
                game->onMouseMove({x, y});
            });

            game->resize(Width, Height);

            while (!glfwWindowShouldClose(window.get()))
            {
                game->update(1.0f / 60.0f);
                game->render();
                glfwSwapBuffers(window.get());
                glfwPollEvents();
            }

            System::shutdown();
        }
    }

    glfwTerminate();
}
