#include "UI.h"

#include "common/AppState.h"
#include "common/ReactiveStateEngine.h"
#include "common/msg.h"
#include "platform/AppMsgQueue.h"

#include "imgui.h"

// Record
// Time signature 4/4
// start
// Input channels
namespace
{

constexpr auto k_targetBufferTimeRange = chr::seconds(1);

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
    const AppState& appState;
    ReactiveStateEngine& rse;

    UIImpl(const AppState& appStateArg, ReactiveStateEngine& rseArg)
        : appState(appStateArg)
        , rse(rseArg)
    {
    }

    void render() override
    {
const        auto& metronome = rse.get(appState.metronome);

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

        double avgUIRefreshHz = NAN;
        double maxDt = NAN;
        double avgDt = NAN;
        if (frameTimes.size() > 1) {
            auto maxDuration = frameTimes[0].dt;
            auto sumDuration = chr::high_resolution_clock::duration::zero();
            auto minT = frameTimes[0].t;
            auto maxT = frameTimes[0].t;
            for (auto& f : frameTimes) {
                maxDuration = std::max(maxDuration, f.dt);
                sumDuration += f.dt;
                minT = std::min(minT, f.t);
                maxT = std::max(maxT, f.t);
            }
            maxDt = chr::duration<double>(maxDuration).count();
            avgDt = chr::duration<double>(sumDuration).count() / frameTimes.size();
            avgUIRefreshHz = (frameTimes.size() - 1) / chr::duration<double>(maxT - minT).count();
        }
        ImGui::TextUnformatted(fmt::format("Average UI refresh: {:.2f} Hz", avgUIRefreshHz).c_str());
        ImGui::TextUnformatted(
          fmt::format("UI repaint time: {:.2f} ms (max {:.2f} ms)", avgDt * 1000, maxDt * 1000).c_str()
        );
        ImGui::Checkbox("Demo Window",
                        &show_demo_window); // Edit bools storing our window open/close state
        bool on = metronome.on;
        if (ImGui::Checkbox("Metronome", &on)) {
            sendToApp(MAKE_VARIANT_V(msg::Metronome, On{on}));
        }
        float bpm = boost::rational_cast<float>(metronome.bpm());
        if (ImGui::SliderFloat("BPM", &bpm, 32, 320, "%.1f", ImGuiSliderFlags_AlwaysClamp)) {
            sendToApp(MAKE_VARIANT_V(msg::Metronome, BPM{bpm}));
        }

        auto outputs = rse.get(appState.outputs);
        if (!outputs.empty()) {
            ImGui::TextUnformatted("Outputs:");
            for (auto& i : outputs) {
                if (ImGui::Checkbox(i.name.c_str(), &i.enabled)) {
                    sendToApp(msg::OutputChanged{i.name, i.enabled});
                }
            }
        }

        auto inputs = rse.get(appState.inputs);
        if (!inputs.empty()) {
            ImGui::TextUnformatted("Inputs:");
            for (auto& i : inputs) {
                if (ImGui::Checkbox(i.name.c_str(), &i.enabled)) {
                    sendToApp(msg::InputChanged{i.name, i.enabled});
                }
            }
        }
        if (!rse.get(appState.recordButtonEnabled)) {
            ImGui::BeginDisabled();
        }
        auto recordButton = rse.get(appState.recordButton);
        if (ImGui::Checkbox("Record", &recordButton)) {
            sendToApp(msg::Transport::record);
        }
        auto clipBeingRecordedSeconds = rse.get(appState.clipBeingRecordedSeconds);
        if (clipBeingRecordedSeconds) {
            ImGui::SameLine();
            ImGui::TextUnformatted(fmt::format("Recording {:.1f} seconds", *clipBeingRecordedSeconds).c_str());
        }
        if (!rse.get(appState.recordButtonEnabled)) {
            ImGui::EndDisabled();
        }

        if (!rse.get(appState.stopButtonEnabled)) {
            ImGui::BeginDisabled();
        }
        auto stopButton = rse.get(appState.stopButton);
        if (ImGui::Checkbox("Stop", &stopButton)) {
            sendToApp(msg::Transport::stop);
        }
        if (!rse.get(appState.stopButtonEnabled)) {
            ImGui::EndDisabled();
        }

        if (!rse.get(appState.playButtonEnabled)) {
            ImGui::BeginDisabled();
        }
        auto playButton = rse.get(appState.playButton);
        if (ImGui::Checkbox("Play", &playButton)) {
            sendToApp(msg::Transport::play);
        }
        auto playedTime = rse.get(appState.playedTime);
        if (playedTime) {
            ImGui::SameLine();
            ImGui::TextUnformatted(fmt::format("Playing {:.1f} seconds", *playedTime).c_str());
        }
        if (!rse.get(appState.playButtonEnabled)) {
            ImGui::EndDisabled();
        }

        auto& clips = rse.get(appState.clips);
        for (size_t i : vi::iota(0u, clips.size())) {
            ImGui::TextUnformatted(fmt::format("Clip #{}", i).c_str());
            ImGui::SameLine(150.0f);
            if (ImGui::Button(fmt::format("Play##{}", i).c_str())) {
                sendToApp(msg::PlayClip{i});
            }
        }

        ImGui::End();

        bool showAudioSettings = rse.get(appState.showAudioSettings);
        if (showAudioSettings) {
            ImGui::Begin(
              "Settings",
              &showAudioSettings,
              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
            );
            if (!showAudioSettings) {
                sendToApp(msg::MainMenu::hideSettings);
            }
            auto& p = rse.get(appState.audioSettingsUI);
            if (ImGui::BeginCombo("Output Device", p.selectedOutputDeviceName().c_str())) {
                for (size_t i : vi::iota(0u, p.outputDeviceNames.size())) {
                    if (ImGui::Selectable(p.outputDeviceNames[i].c_str(), i == p.selectedOutputDeviceIx)) {
                        sendToApp(MAKE_VARIANT_V(msg::AudioSettings, OutputDeviceSelected{i}));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Input Device", p.selectedInputDeviceName().c_str())) {
                for (size_t i : vi::iota(0u, p.inputDeviceNames.size())) {
                    if (ImGui::Selectable(p.inputDeviceNames[i].c_str(), i == p.selectedInputDeviceIx)) {
                        sendToApp(MAKE_VARIANT_V(msg::AudioSettings, InputDeviceSelected{i}));
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::End();
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

        ImGui::SetNextWindowPos(ImVec2(400, 0));
        ImGui::SetNextWindowSize(ImVec2(400, io.DisplaySize.y));
        ImGui::Begin(
          "Arrangement",
          nullptr,
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings
        );

        auto& arr = rse.get(appState.arrangement);
        int sectionIx = 0;
        constexpr float k_pixelsPerSeconds = 40.0f;
        for (auto& a : arr.sections) {
            auto secondsOfSection = boost::rational_cast<float>(a.duration(metronome.tempo));
            ImGui::BeginChild(
              fmt::format("ArrangementSection{}", sectionIx++).c_str(),
              ImVec2(ImGui::GetContentRegionAvail().x, secondsOfSection * k_pixelsPerSeconds),
              ImGuiChildFlags_Borders,
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            );
            auto tempo = a.tempo.value_or(metronome.tempo);
            ImGui::TextUnformatted(fmt::format("name: {}, {} s", a.name, secondsOfSection).c_str());
            const auto windowPos = ImGui::GetWindowPos();
            switch_variant(
              a.structure,
              [&](const Bars& x) {
                  float secondsInBars = 0;
                  for (auto& b : x.bars) {
                      ImGui::TextUnformatted(fmt::format("{}/{}", b.timeSignature.upper, b.timeSignature.lower).c_str()
                      );
                      const auto beatInSeconds = boost::rational_cast<float>(60 / tempo / b.timeSignature.lower);
                      bool firstInBar = true;
                      for (UNUSED auto beatIxInBar : vi::iota(0, b.timeSignature.upper)) {
                          auto y = secondsInBars * k_pixelsPerSeconds;
                          if (firstInBar) {
                              ImGui::GetWindowDrawList()->AddLine(
                                windowPos + ImVec2(190, y), windowPos + ImVec2(210, y), IM_COL32_WHITE, 2
                              );
                              firstInBar = false;
                          } else {
                              ImGui::GetWindowDrawList()->AddCircleFilled(
                                windowPos + ImVec2(200, y), 1, IM_COL32_WHITE
                              );
                          }
                          secondsInBars += beatInSeconds;
                      }
                  }
              },
              [&](const Period& x) {
                  ImGui::TextUnformatted(fmt::format("{:.2f} whole notes", boost::rational_cast<float>(x.wholeNotes)).c_str());
                  auto numWholeBeats = (x.wholeNotes.numerator() * metronome.timeSignature.lower) / x.wholeNotes.denominator() + 1;
                  for (int64_t i : vi::iota(0, numWholeBeats)) {
                      auto y = boost::rational_cast<float>(Rational(i) / tempo * 60) * k_pixelsPerSeconds;
                      ImGui::GetWindowDrawList()->AddCircleFilled(windowPos + ImVec2(200, y), 1, IM_COL32_WHITE);
                  }
              },
              [](const Duration& x) {
                  ImGui::TextUnformatted(fmt::format("{:.2f} seconds", boost::rational_cast<float>(x.seconds)).c_str()
                  );
              }
            );

            ImGui::EndChild();
        }
        ImGui::End();

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
    struct FrameTimeItem {
        chr::high_resolution_clock::time_point t;
        chr::high_resolution_clock::duration dt;
    };
    vector<FrameTimeItem> frameTimes;
    size_t nextFrameTimeIx = 0;
    void addFrameTime(chr::high_resolution_clock::time_point t, chr::high_resolution_clock::duration dt) override
    {
        // Maintain frameTimes as a circular buffer.
        if (frameTimes.size() <= nextFrameTimeIx) {
            frameTimes.push_back(FrameTimeItem{t, dt});
            nextFrameTimeIx = frameTimes.size();
        } else {
            frameTimes[nextFrameTimeIx++] = FrameTimeItem{t, dt};
        }
        auto bufferTimeRange = t - frameTimes[0].t;
        if (bufferTimeRange > k_targetBufferTimeRange) {
            frameTimes.resize(nextFrameTimeIx);
            nextFrameTimeIx = 0;
        }
    }
};

unique_ptr<UI> UI::make(const AppState& appState, ReactiveStateEngine& rse)
{
    return make_unique<UIImpl>(appState, rse);
}
