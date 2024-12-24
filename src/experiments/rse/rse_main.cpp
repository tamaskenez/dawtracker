#include "common/ReactiveStateEngine.h"
#include "common/common.h"

struct State {
    int a, b;
    int c;
};

int main()
{
    State s;
    ReactiveStateEngine rse(&s);
    rse.registerUpdater(s.c, [&s, &rse]() {
        return rse.get(s.a) + rse.get(s.b);
    });
    rse.set(s.a, 1);
    rse.set(s.b, 2);
    fmt::println("c: {}", rse.get(s.c));
    rse.set(s.a, 10);
    fmt::println("c: {}", rse.get(s.c));

    return EXIT_SUCCESS;
}
