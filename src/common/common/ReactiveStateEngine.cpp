#include "ReactiveStateEngine.h"

void rse::NodeBase::setTimestampAndMarkDownstreamNodesOutOfDate(uint64_t newTimestamp)
{
    timestamp = newTimestamp;
    for (auto* d : downstreamNodes) {
        d->markThisAndDownstreamNodesOutOfDate();
    }
}

void rse::ComputedNodeBase::markThisAndDownstreamNodesOutOfDate()
{
    // We assume that if this is not up-to-date then no downstream nodes will up-to-date since no node can be up-to-date
    // without first makeing sure its upstream nodes are up-to-date.
    if (upToDate) {
        upToDate = false;
        for (auto* d : downstreamNodes) {
            d->markThisAndDownstreamNodesOutOfDate();
        }
    }
}

void rse::NodeBase::addDownstreamNode(rse::ComputedNodeBase* cnb)
{
    auto it = ra::lower_bound(downstreamNodes, cnb);
    CHECK(it == downstreamNodes.end() || *it != cnb);
    downstreamNodes.insert(it, cnb);
}

rse::ScopedUndoables::~ScopedUndoables()
{
    if (that) {
        that->finishUndoables();
    }
}

void ReactiveStateEngine::addToInputCollectorIfNeeded(const rse::NodeBase& nb)
{
    if (inputCollectorDuringRegistration) {
        inputCollectorDuringRegistration->push_back(&nb);
    }
}

bool ReactiveStateEngine::updateIfNeededCore(const rse::ComputedNodeBase& cnb)
{
    if (inputCollectorDuringRegistration) {
        inputCollectorDuringRegistration->push_back(&cnb);
        return false;
    }
    return updateIfNeededCore2(cnb);
}

bool ReactiveStateEngine::updateIfNeededCore2(const rse::ComputedNodeBase& vk)
{
    if (vk.upToDate) {
        // This might be an input variable (no updateFn, always up-to-date) or normal variable that happens to be
        // up-to-date.
#ifndef NDEBUG
        // If this is up-to-date, the upstream variables
        // - must be all up-to-date
        // - they must not have a timestamp greater than upstreamProcessedUntilTimestamp.
        for (auto& uv : vk.upstreamNodes) {
            switch_variant(
              uv,
              [&](const rse::NodeBase* x) {
                  CHECK(x->timestamp <= vk.upstreamProcessedUntilTimestamp);
              },
              [&](const rse::ComputedNodeBase* x) {
                  CHECK(x->upToDate && x->timestamp <= vk.upstreamProcessedUntilTimestamp);
              }
            );
        }
#endif
        return false;
    }

    CHECK(vk.computeAndUpdateIfDifferentFn);

#ifndef NDEBUG
    // If this is not up-to-date, the downstream variables must be out-of-date, too.
    for (auto* dv : vk.downstreamNodes) {
        CHECK(!dv->upToDate);
    }
#endif

    // Make sure all upstream dependencies are up-to-date.
    auto maxUpstreamTimestamp = vk.upstreamProcessedUntilTimestamp;
    for (auto& uv : vk.upstreamNodes) {
        switch_variant(
          uv,
          [&](const rse::NodeBase* x) {
              if (maxUpstreamTimestamp < x->timestamp) {
                  maxUpstreamTimestamp = x->timestamp;
              }
          },
          [&](const rse::ComputedNodeBase* x) {
              if (updateIfNeededCore2(*x) || maxUpstreamTimestamp < x->timestamp) {
                  assert(maxUpstreamTimestamp < x->timestamp);
                  maxUpstreamTimestamp = x->timestamp;
              }
          }
        );
    }
    bool changed = vk.upstreamProcessedUntilTimestamp < maxUpstreamTimestamp && vk.computeAndUpdateIfDifferentFn();
    auto& vkMutable = const_cast<rse::ComputedNodeBase&>(vk);
    if (changed) {
        vkMutable.timestamp = nextTimestamp++;
    }
    vkMutable.upToDate = true;
    vkMutable.upstreamProcessedUntilTimestamp = maxUpstreamTimestamp;
    return changed;
}

void ReactiveStateEngine::registerUpdaterCore_prepare(rse::ComputedNodeBase& v)
{
    CHECK(!v.computeAndUpdateIfDifferentFn);
    CHECK(!inputCollectorDuringRegistration);
    inputCollectorDuringRegistration.emplace();
}

void ReactiveStateEngine::registerUpdaterCore_finalize(rse::ComputedNodeBase& v)
{
    v.upstreamNodes = MOVE(inputCollectorDuringRegistration.value());
    inputCollectorDuringRegistration.reset();
    sortUniqueInplace(v.upstreamNodes);
    for (auto& uv : v.upstreamNodes) {
        const_cast<rse::NodeBase*>(switch_variant(
                                     uv,
                                     [](const rse::NodeBase* x) {
                                         return x;
                                     },
                                     [](const rse::ComputedNodeBase* x) -> const rse::NodeBase* {
                                         return x;
                                     }
                                   )
        )->addDownstreamNode(&v);
    }
}

rse::ScopedUndoables ReactiveStateEngine::beginUndoables()
{
    CHECK(!undoablesCollector);
    undoablesCollector.emplace();
    return rse::ScopedUndoables(this);
}

void ReactiveStateEngine::finishUndoables()
{
    CHECK(undoablesCollector);
    undoablesCollector->executeRedo();
    if (!undoablesCollector->undoRedoOps.empty()) {
        undoRedoHistory.push_back(UndoRedoNode(MOVE(*undoablesCollector)));
    }
    undoablesCollector.reset();
}

ReactiveStateEngine::UndoRedoNodeBase::UndoRedoNodeBase(function<void()> undoFn, function<void()> redoFn)
    : undoRedoOps({pair(MOVE(undoFn), MOVE(redoFn))})
{
}

void ReactiveStateEngine::UndoRedoNodeBase::pushBack(function<void()> undoFn, function<void()> redoFn)
{
    undoRedoOps.push_back(pair(MOVE(undoFn), MOVE(redoFn)));
}

void ReactiveStateEngine::UndoRedoNodeBase::executeUndo()
{
    for (auto it = undoRedoOps.rbegin(); it != undoRedoOps.rend(); ++it) {
        it->first();
    }
}
void ReactiveStateEngine::UndoRedoNodeBase::executeRedo()
{
    for (auto& op : undoRedoOps) {
        op.second();
    }
}

ReactiveStateEngine::UndoRedoNode::UndoRedoNode(UndoRedoNodeBase base)
    : UndoRedoNodeBase(MOVE(base))
    , systemTimePoint(chr::system_clock::now())
    , steadyTimePoint(chr::steady_clock::now())
{
}

ReactiveStateEngine::UndoRedoNode::UndoRedoNode(function<void()> undoFn, function<void()> redoFn)
    : UndoRedoNodeBase(MOVE(undoFn), MOVE(redoFn))
    , systemTimePoint(chr::system_clock::now())
    , steadyTimePoint(chr::steady_clock::now())
{
}

void ReactiveStateEngine::undoableOpReceived(function<void()> undoFn, function<void()> redoFn)
{
    if (undoablesCollector) {
        undoablesCollector->pushBack(MOVE(undoFn), MOVE(redoFn));
    } else {
        redoFn();
        undoRedoHistory.erase(nextNodeToRedo, undoRedoHistory.end());
        undoRedoHistory.push_back(UndoRedoNode(MOVE(undoFn), MOVE(redoFn)));
        nextNodeToRedo = undoRedoHistory.end();
    }
}
