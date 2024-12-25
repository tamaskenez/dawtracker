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
    return updateIfNeededCore2(variables[koffset]);
}

bool ReactiveStateEngine::updateIfNeededCore2(Variable& vk)
{
    if (vk.upToDate) {
        // This might be an input variable (no updateFn, always up-to-date) or normal variable that happens to be
        // up-to-date.
#ifndef NDEBUG
        // If this is up-to-date, the upstream variables
        // - must be all up-to-date
        // - they must not have a timestamp greater than upstreamProcessedUntilTimestamp.
        for (auto uv : vk.upstreamVariables) {
            auto& vuv = variables[uv];
            CHECK(vuv.upToDate && vuv.timestamp <= vk.upstreamProcessedUntilTimestamp);
        }
#endif
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
    auto maxUpstreamTimestamp = vk.upstreamProcessedUntilTimestamp;
    for (auto uv : vk.upstreamVariables) {
        auto& vuv = variables[uv];
        if (updateIfNeededCore2(vuv) || maxUpstreamTimestamp < vuv.timestamp) {
            assert(maxUpstreamTimestamp < vuv.timestamp);
            maxUpstreamTimestamp = vuv.timestamp;
        }
    }
    bool changed = vk.upstreamProcessedUntilTimestamp < maxUpstreamTimestamp && vk.updateFn(vk);
    vk.upToDate = true;
    vk.upstreamProcessedUntilTimestamp = maxUpstreamTimestamp;
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
    v.timestamp = 0;

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
