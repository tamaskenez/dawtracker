#include "UI.h"

#include "common/msg.h"

#include "imgui.h"

namespace
{

string makeMenuShortcutString(string_view s)
{
    const char* prefix{};
    switch (getPlatform()) {
    case Platform::mac:
        prefix = "⌘";
        break;
    case Platform::win:
        prefix = "Ctrl+";
        break;
    }
    return fmt::format("{}{}", prefix, s);
}
} // namespace

struct UIImpl : public UI {
    bool show_demo_window = false;
    struct Dialogs {
        optional<const uistate::AudioSettings*> audioSettings;
    } dialogs;
    void render() override
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::Begin(
          "MainWindow",
          nullptr,
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar
        );

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("DawTracker")) {
                if (ImGui::MenuItem("Settings", makeMenuShortcutString(",").c_str())) {
                    sendToApp(msg::MainMenu::settings);
                }
                if (ImGui::MenuItem("Quit", makeMenuShortcutString("Q").c_str())) {
                    sendToApp(msg::MainMenu::quit);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::Checkbox("Demo Window",
                        &show_demo_window); // Edit bools storing our window open/close state
        ImGui::End();

        bool dialogs_audioSettings_has_value = dialogs.audioSettings.has_value();
        if (dialogs_audioSettings_has_value) {
            ImGui::Begin(
              "Settings",
              &dialogs_audioSettings_has_value,
              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
            );
            auto* p = (*dialogs.audioSettings);
            if (ImGui::BeginCombo("Output Device", p->selectedOutputDeviceName().c_str())) {
                for (size_t i : vi::iota(0u, p->outputDeviceNames.size())) {
                    if (ImGui::Selectable(p->outputDeviceNames[i].c_str(), i == p->selectedOutputDeviceIx)) {
                        sendToApp(MAKE_VARIANT_V(msg::AudioSettings, OutputDeviceSelected{i}));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Input Device", p->selectedInputDeviceName().c_str())) {
                for (size_t i : vi::iota(0u, p->inputDeviceNames.size())) {
                    if (ImGui::Selectable(p->inputDeviceNames[i].c_str(), i == p->selectedInputDeviceIx)) {
                        sendToApp(MAKE_VARIANT_V(msg::AudioSettings, InputDeviceSelected{i}));
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::End();
        }
        if (!dialogs_audioSettings_has_value) {
            dialogs.audioSettings.reset();
        }

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
        // ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair
        // to create a named window.
        if ((false)) {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                           // and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can
                                                      // use a format strings too)
            ImGui::Checkbox("Demo Window",
                            &show_demo_window); // Edit bools storing our window open/close state

            ImGui::SliderFloat("float", &f, 0.0f,
                               1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

            // ImGui::ColorEdit3("clear color",
            //                   (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                         // widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        // if (show_another_window) {
        //    ImGui::Begin(
        //      "Another Window",
        //      &show_another_window
        //    ); // Pass a pointer to our bool variable (the
        //       // window will have a closing button that will
        //       // clear the bool when clicked)
        //    ImGui::Text("Hello from another window!");
        //    if (ImGui::Button("Close Me"))
        //        show_another_window = false;
        //    ImGui::End();
        //}

        // Rendering
        ImGui::Render();
    }
    void openSettings(const uistate::AudioSettings* audioSettingsState) override
    {
        dialogs.audioSettings = audioSettingsState;
    }
    void closeDialogs() override
    {
        dialogs = Dialogs{};
    }
};

unique_ptr<UI> UI::make()
{
    return make_unique<UIImpl>();
}
