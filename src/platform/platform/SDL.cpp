#include "SDL.h"

#include "common/common.h"

// Initialization code is from ImGui sdl3+opengl3 backend example.
// https://github.com/ocornut/imgui/blob/master/examples/example_sdl3_opengl3/main.cpp

#include <SDL3/SDL.h>
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
  #include <SDL3/SDL_opengles2.h>
#else
  #include <SDL3/SDL_opengl.h>
#endif

// This example doesn't compile with Emscripten yet! Awaiting SDL3 support.
#ifdef __EMSCRIPTEN__
  #include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

struct SDLImpl : public SDL {
    SDL_Window* _window{};
    SDL_GLContext _gl_context{};
    const char* _glsl_version{};

    SDLImpl()
    {
        // Setup SDL
        LOG_IF(FATAL, !SDL_Init(SDL_INIT_VIDEO)) << fmt::format("Error: SDL_Init(): {}", SDL_GetError());

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        _glsl_version = "#version 100";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
        // GL 3.2 Core + GLSL 150
        _glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
        // GL 3.0 + GLSL 130
        _glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
        _window = SDL_CreateWindow("DawTracker", 1280, 720, window_flags);
        LOG_IF(FATAL, !_window) << fmt::format("Error: SDL_CreateWindow(): {}", SDL_GetError());
        SDL_SetWindowPosition(_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        _gl_context = SDL_GL_CreateContext(_window);
        SDL_GL_MakeCurrent(_window, _gl_context);
        SDL_GL_SetSwapInterval(1); // Enable vsync
        SDL_ShowWindow(_window);
    }
    ~SDLImpl() override
    {
        SDL_GL_DestroyContext(_gl_context);
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    SDL_Window* window() const override
    {
        return _window;
    }
    void* gl_context() const override
    {
        return _gl_context;
    }
    const char* glsl_version() const override
    {
        return _glsl_version;
    }
};

unique_ptr<SDL> SDL::init()
{
    return make_unique<SDLImpl>();
}
