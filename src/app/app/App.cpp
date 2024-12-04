#include "App.h"

#include "ui/UI.h"

struct AppImpl : public App {
    UI* ui;
    explicit AppImpl(UI* uiArg)
        : ui(uiArg)
    {
    }
};

unique_ptr<App> App::make(UI* ui)
{
    return make_unique<AppImpl>(ui);
}
