#include "ReactiveStateEngine.h"

void ReactiveStateEngine::markThisAndTransitiveDependenciesOutOfDate(intptr_t offset)
{
    auto& v = variables[offset];
    if (v.upToDate) {
        v.upToDate = false;
        for (auto d : v.downstreamVariables) {
            markThisAndTransitiveDependenciesOutOfDate(d);
        }
    }
}

void ReactiveStateEngine::Variable::addDownstreamVariable(intptr_t d)
{
    auto it = ra::lower_bound(downstreamVariables, d);
    CHECK(it == downstreamVariables.end() || *it != d);
    downstreamVariables.insert(it, d);
}

bool ReactiveStateEngine::updateIfNeededCore(intptr_t koffset)
{
    if (inputCollectorDuringRegistration) {
        inputCollectorDuringRegistration->push_back(koffset);
        return false;
    }
    auto& vk = variables[koffset];

    if (vk.upToDate) {
        // This might be an input variable (no updateFn, always up-to-date) or normal variable that happens to be
        // up-to-date.
        return false;
    }

    CHECK(vk.updateFn);

#ifndef NDEBUG
    // If this is not up-to-date, the downstream variables must be out-of-date, too.
    for (auto dv : vk.downstreamVariables) {
        CHECK(!variables[dv].upToDate);
    }
#endif

    // Make sure all upstream dependencies are up-to-date.
    // TODO: reintroduce timestamps and initialize needToCallUpdater with false.
    bool needToCallUpdater = true;
    for (auto uv : vk.upstreamVariables) {
        if (updateIfNeededCore(uv)) {
            needToCallUpdater = true;
        }
    }
    bool changed = needToCallUpdater && vk.updateFn();
    vk.upToDate = true;
    return changed;
}

void ReactiveStateEngine::markChangedPrivate(intptr_t offset)
{
    auto& v = variables[offset];
    for (auto d : v.downstreamVariables) {
        markThisAndTransitiveDependenciesOutOfDate(d);
    }
}

ReactiveStateEngine::Variable& ReactiveStateEngine::registerUpdaterCore_prepare(intptr_t offset)
{
    auto& v = variables[offset];
    CHECK(!v.updateFn);

    v.upToDate = false;

    CHECK(!inputCollectorDuringRegistration);
    inputCollectorDuringRegistration.emplace();

    return v;
}

void ReactiveStateEngine::registerUpdaterCore_finalize(intptr_t offset, Variable& v)
{
    v.upstreamVariables = MOVE(inputCollectorDuringRegistration.value());
    inputCollectorDuringRegistration.reset();
    ra::sort(v.upstreamVariables);
    CHECK(ra::adjacent_find(v.upstreamVariables) == v.upstreamVariables.end());
    for (auto uv : v.upstreamVariables) {
        variables[uv].addDownstreamVariable(offset);
    }
}
