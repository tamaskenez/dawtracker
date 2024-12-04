#include "UI.h"

struct UIImpl : public UI {
};

unique_ptr<UI> UI::make()
{
    return make_unique<UIImpl>();
}
