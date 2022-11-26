#include "log.h"
#include "game.h"
#include "system.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>

int main()
{
    constexpr auto Width = 1200;
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
            System::instance().initialize();

            auto game = std::make_unique<Game>();
            game->resize(Width, Height);

            while (!glfwWindowShouldClose(window.get()))
            {
                game->render();
                glfwSwapBuffers(window.get());
                glfwPollEvents();
            }

            System::instance().release();
        }
    }

    glfwTerminate();
}
