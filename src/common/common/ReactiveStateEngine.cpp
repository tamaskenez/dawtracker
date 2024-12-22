#include "ReactiveStateEngine.h"

void ReactiveStateEngine::bumpThisAndDownstreamVariablesMaxUpstreamTimestamp(intptr_t offset, uint64_t timestamp)
{
    auto& v = variables[offset];
    assert(v.maxUpstreamTimestamp <= timestamp);
    if (v.maxUpstreamTimestamp < timestamp) {
        v.maxUpstreamTimestamp = timestamp;
        for (auto d : v.dependents) {
            bumpThisAndDownstreamVariablesMaxUpstreamTimestamp(d, timestamp);
        }
    }
}

bool ReactiveStateEngine::Variable::addDependent(intptr_t d)
{
    auto it = ra::lower_bound(dependents, d);
    if (it != dependents.end() && *it == d) {
        return false;
    }
    dependents.insert(it, d);
    return true;
}
