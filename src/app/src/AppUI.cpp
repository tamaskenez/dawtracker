#include "AppUI.h"

#include "common/common.h"

bool AppUI::receive(std::any&& msg)
{
    for (auto& d : dialogs) {
        if (d->receive(MOVE(msg))) {
            return true;
        }
    }
    return false;
}
