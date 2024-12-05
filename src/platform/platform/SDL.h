#include "common/std.h"

struct SDL_Window;

class SDL
{
public:
    static unique_ptr<SDL> init();
    virtual ~SDL() = default;

    virtual SDL_Window* window() const = 0;
    virtual void* gl_context() const = 0;
    virtual const char* glsl_version() const = 0;
};
