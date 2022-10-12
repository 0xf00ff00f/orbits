#include "log.h"
#include "game.h"

#include <GL/glew.h>

#ifdef USE_GLFW3
#include <GLFW/glfw3.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <memory>

int main()
{
    constexpr auto Width = 1200;
    constexpr auto Height = 600;

#ifdef USE_GLFW3
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
            auto game = std::make_unique<Game>();
            game->resize(Width, Height);

            while (!glfwWindowShouldClose(window.get()))
            {
                game->render();
                glfwSwapBuffers(window.get());
                glfwPollEvents();
            }
        }
    }

    glfwTerminate();
#else
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
        panic("Failed to initialize SDL\n");

    {
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN),
            SDL_DestroyWindow);
        if (!window)
            panic("Failed to create window: %s\n", SDL_GetError());

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        std::unique_ptr<std::remove_pointer<SDL_GLContext>::type, decltype(&SDL_GL_DeleteContext)> context(
            SDL_GL_CreateContext(window.get()), SDL_GL_DeleteContext);
        if (!context)
            panic("Failed to create context: %s\n", SDL_GetError());

        SDL_GL_MakeCurrent(window.get(), context.get());
        SDL_GL_SetSwapInterval(1);

        glewInit();

        log("Vendor: %s\n", glGetString(GL_VENDOR));
        log("Renderer: %s\n", glGetString(GL_RENDERER));
        log("Version: %s\n", glGetString(GL_VERSION));

        {
            auto game = std::make_unique<Game>();
            game->resize(Width, Height);

            bool done = false;
            while (!done)
            {
                assert(glGetError() == GL_NO_ERROR);

                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    switch (event.type)
                    {
                    case SDL_QUIT:
                        done = true;
                        break;
                    }
                }

                game->render();
                SDL_GL_SwapWindow(window.get());
            }
        }
    }

    SDL_Quit();
#endif
}
