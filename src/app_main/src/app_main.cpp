#include "app/App.h"
#include "common/AppState.h"
#include "common/ReactiveStateEngine.h"
#include "common/common.h"
#include "common/msg.h"
#include "platform/AppMsgQueue.h"
#include "platform/SDL.h"
#include "platform/platform.h"
#include "ui/UI.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
  #include <SDL3/SDL_opengles2.h>
#else
  #include <SDL3/SDL_opengl.h>
#endif

namespace
{
constexpr auto k_minUIRefreshInterval = chr::milliseconds(50);
constexpr auto k_maxUIRefreshInterval = chr::milliseconds(200);
constexpr double k_refreshIntervalIncreaseFactorWhenNoEvents = 1.05;
} // namespace

//// This example doesn't compile with Emscripten yet! Awaiting SDL3 support.
// #ifdef __EMSCRIPTEN__
//   #include "../libs/emscripten/emscripten_mainloop_stub.h"
// #endif

// Main code
int main(int, char**)
{
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfo);

    auto sdl = SDL::init();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(sdl->window(), sdl->gl_context());
    ImGui_ImplOpenGL3_Init(sdl->glsl_version());

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please
    // handle those errors in your application (e.g. use an assertion, or display
    // an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored
    // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    // ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype
    // for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at
    // runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

    static const ImWchar glyphRange[] = {32, 255, 0x2310, 0x2320, 0};
    // TODO: Bundle some font with the application.
    io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/HelveticaNeue.ttc", 18, nullptr, glyphRange);
    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    AppState appState;
    ReactiveStateEngine rse;
    auto ui = UI::make(appState, rse);
    auto app = App::make(ui.get(), appState, rse);

    auto amq = AppMsgQueue::make([app_ = app.get()](std::any&& msg) {
        app_->receive(MOVE(msg));
    });
    amq->makeThisGlobalAppQueue(true);

    // Main loop
    bool done = false;
    optional<chr::high_resolution_clock::time_point> lastUIRefreshStartTime;
    auto targetRefreshInterval = chr::duration<double>(k_minUIRefreshInterval);
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not
    // attempt to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
        // your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application, or clear/overwrite your copy of the
        // keyboard data. Generally you may always pass all inputs to dear imgui,
        // and hide them from your application based on those two flags.
        SDL_Event event;
        bool hadAnSDLEvent = false;
        for (bool waitForEvent = false;;) {
            if (waitForEvent) {
                if (!lastUIRefreshStartTime) {
                    break;
                }
                auto timeout = targetRefreshInterval - (chr::high_resolution_clock::now() - *lastUIRefreshStartTime);
                if (timeout < chr::milliseconds(1)) {
                    break;
                }
                auto timeoutMs = clampCast<Sint32>(chr::duration_cast<chr::milliseconds>(timeout).count());
                if (!SDL_WaitEventTimeout(&event, timeoutMs)) {
                    break;
                }
            } else {
                if (!SDL_PollEvent(&event)) {
                    waitForEvent = true;
                    continue;
                }
            }
            hadAnSDLEvent = true;
            if (event.type == appQueueNotificationSdlEventType()) {
                tryDequeueAndMakeAppReceiveIt();
            } else {
                static int counter = 0;
                fmt::println("sdl event {} {}", event.type, counter++);
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT) {
                    done = true;
                }
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED
                    && event.window.windowID == SDL_GetWindowID(sdl->window())) {
                    done = true;
                }
            }
        }
        app->runAudioIODispatchLoop();
        lastUIRefreshStartTime = chr::high_resolution_clock::now();
        if (SDL_GetWindowFlags(sdl->window()) & SDL_WINDOW_MINIMIZED) {
            continue;
        }
        if (app->getAndClearIfUIRefreshNeeded() || hadAnSDLEvent) {
            targetRefreshInterval = k_minUIRefreshInterval;
        } else {
            targetRefreshInterval = std::min(
              targetRefreshInterval * k_refreshIntervalIncreaseFactorWhenNoEvents,
              chr::duration<double>(k_maxUIRefreshInterval)
            );
        }
        // Render UI frame.
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ui->render();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClearColor(
              clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w
            );
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(sdl->window());
            auto t1 = chr::high_resolution_clock::now();
            ui->addFrameTime(*lastUIRefreshStartTime, t1 - *lastUIRefreshStartTime);
        }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    amq->makeThisGlobalAppQueue(false);
    amq.reset();

    amq = AppMsgQueue::make([](std::any&& msg) {
        LOG(INFO) << fmt::format("Discarded message to app because it's being destructed: {}", msg.type().name());
    });
    amq->makeThisGlobalAppQueue(true);
    app.reset();
    ui.reset();
    amq->makeThisGlobalAppQueue(false);

    return 0;
}
