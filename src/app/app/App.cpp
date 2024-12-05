#include "App.h"

#include "common/msg.h"
#include "platform/Msg.h"
#include "platform/platform.h"
#include "ui/UI.h"

struct AppImpl : public App {
    UI* ui;
    explicit AppImpl(UI* uiArg)
        : ui(uiArg)
    {
    }
    void receive(Msg&& msg) override
    {
        auto pl = MOVE(msg.payload);
        if (auto* a = std::any_cast<msg::MainMenu>(&pl)) {
            switch (*a) {
            case msg::MainMenu::quit:
                sendQuitEventToAppMain();
                break;
            }
        } else {
            LOG(FATAL) << fmt::format("Invalid message: {}", pl.type().name());
        }
    }
};

unique_ptr<App> App::make(UI* ui)
{
    return make_unique<AppImpl>(ui);
}
