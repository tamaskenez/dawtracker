#pragma once

#include "common.h"

// Manages a (possibly nested) struct such that
//
// - the user can register update functions for the fields
// - the update functions query other fields in the struct and return the new value of the field they compute the value
// for
// - this engine detects which field was needed to compute the value of which field
// - when one field changes, automatically marks dependent fields dirty
// - setting and getting the values of the fields must be done through the get/set methods of the engine
//
// Example:
//
//     struct State {
//       int a, b, c;
//     } state;
//
//     ReactiveStateEngine rse(&s);
//     rse.registerUpdater(state.c, [&state, &rse]() {
//         return rse.get(state.a) + rse.get(state.b);
//     });
//     rse.set(state.a, 1);
//     rse.set(state.b, 2);
//     fmt::println("c: {}", rse.get(state.c)); // prints 3
//

class ReactiveStateEngine;

namespace rse
{

class ComputedNodeBase;

class NodeBase
{
    friend class ::ReactiveStateEngine;

protected:
    NodeBase() = default;
    // One node can only be added only once.
    void addDownstreamNode(ComputedNodeBase* downstreamNode);
    void setTimestampAndMarkDownstreamNodesOutOfDate(uint64_t newTimestamp);

    uint64_t timestamp = 1;
    vector<ComputedNodeBase*> downstreamNodes; // Variables dependending on this variable.
};

class ComputedNodeBase;
using UpstreamNode = variant<const rse::NodeBase*, rse::ComputedNodeBase*>;

class ComputedNodeBase : public NodeBase
{
    friend class ::ReactiveStateEngine;
    friend class NodeBase;

protected:
    ComputedNodeBase() = default;
    void markThisAndDownstreamNodesOutOfDate();

    bool upToDate = false;
    // If the input nodes' timestamps are not greater than this value, this node doesn't need to be recomputed and
    // can be marked up to date.
    uint64_t upstreamProcessedUntilTimestamp = 0;
    vector<UpstreamNode> upstreamNodes; // Nodes which this nodes's computeAndUpdateIfDifferentFn uses as input.
    function<bool()> computeAndUpdateIfDifferentFn; // Return true if changed.
};

// An inner node in the graph, it holds a value which can only be changed through a registered updater function which is
// invoked automatically when the node's inputs, the upstream nodes have been changed.
template<class V>
class Computed : public ComputedNodeBase
{
private:
    friend class ::ReactiveStateEngine;
    V v;
};

// A node whose value is set from the outside. It has simple value semantics, can be set (through ReactiveStateEngine)
// as a whole.
template<class V>
class Value : public NodeBase
{
public:
    Value() = default;
    explicit Value(V vArg)
        : v(MOVE(vArg))
    {
    }

private:
    friend class ::ReactiveStateEngine;
    V v;
};

// Like Value but undoable.
template<class V>
class UndoableValue : public NodeBase
{
public:
    UndoableValue() = default;
    explicit UndoableValue(V vArg)
        : v(MOVE(vArg))
    {
    }

private:
    friend class ::ReactiveStateEngine;
    V v;
};

template<class T>
UpstreamNode asUpstreamNodePointer(T& x)
{
    static_assert(std::is_base_of_v<NodeBase, T>, "Only types derived from NodeBase are allowed.");
    if constexpr (std::is_base_of_v<ComputedNodeBase, T>) {
        auto* nonConstXPointer = const_cast<std::remove_const_t<T>*>(&x);
        return static_cast<ComputedNodeBase*>(nonConstXPointer);
    } else {
        return static_cast<const NodeBase*>(&x);
    }
}

class ScopedUndoables
{
    friend class ::ReactiveStateEngine;
    ::ReactiveStateEngine* that;
    ScopedUndoables(::ReactiveStateEngine* thatArg)
        : that(thatArg)
    {
    }

public:
    ~ScopedUndoables();
};

} // namespace rse

class ReactiveStateEngine
{
public:
    template<class V, class Fn>
    void registerUpdater(rse::Computed<V>& k, Fn computeFn)
    {
        auto fn = function<V()>(MOVE(computeFn));
        registerUpdaterCore(k, fn, nullopt);
    }

    template<class V, class Fn, class... UpstreamNodes>
    void registerUpdater(rse::Computed<V>& k, Fn computeFn, const UpstreamNodes&... upstreamNodes)
    {
        registerUpdaterCore(
          k,
          function<V()>(MOVE(computeFn)),
          initializer_list<rse::UpstreamNode>{rse::asUpstreamNodePointer(upstreamNodes)...}
        );
    }

    // Return true if it had to be updated (the value has changed during the update).
    template<class V>
    bool updateIfNeeded(const rse::Computed<V>& k)
    {
        return updateIfNeededCore(k);
    }

    // Query the value of k. Might call the updater function.
    template<class V>
    const V& get(const rse::Computed<V>& k)
    {
        updateIfNeededCore(const_cast<rse::Computed<V>&>(k));
        return k.v;
    }
    template<class V>
    const V& get(const rse::Value<V>& k)
    {
        addToInputCollectorIfNeeded(k);
        return k.v;
    }
    template<class V>
    const V& get(const rse::UndoableValue<V>& k)
    {
        addToInputCollectorIfNeeded(k);
        return k.v;
    }

    bool isUpToDate(const rse::ComputedNodeBase& k)
    {
        return k.upToDate;
    }

    // Assign new value to the variable, and mark all transitive dependencies outdated if the new value is different
    // from the current one.
    template<class K, class V>
    bool set(rse::Value<K>& k, V&& newValue)
    {
        if (newValue == k.v) {
            return false;
        }
        k.v = std::forward<V>(newValue);
        k.setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        return true;
    }
    // Like `set` but do not compare new value to the existing value, always assume that it has changed.
    // `setAsDifferent` is a useful alternative to `set` if we don't want to call the equality operator for the type
    // (because it doesn not exist or expensive).
    template<class K, class V>
    void setAsDifferent(rse::Value<K>& k, V&& newValue)
    {
        k.v = std::forward<V>(newValue);
        k.setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
    }
    // Set new value and return old value.
    template<class K, class V>
    K exchange(rse::Value<K>& k, V&& newValue)
    {
        if (newValue == k.v) {
            return newValue;
        }
        auto oldValue = MOVE(k.v);
        k.v = std::forward<V>(newValue);
        k.setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        return oldValue;
    }
    // Assume K is a map with std interface. Return the return value of K::insert()
    template<class K, class Key, class Value>
    auto insert(rse::Value<K>& k, pair<Key, Value> newValue)
    {
        auto itb = k.v.insert(MOVE(newValue));
        if (itb.second) {
            k.setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        }
        return itb;
    }
    // Assume K is a container with push_back.
    template<class K, class Value>
    void pushBack(rse::Value<K>& k, Value&& newValue)
    {
        k.v.push_back(std::forward<Value>(newValue));
        k.setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
    }
    // Assume K is a map with std interface.
    template<class K, class Key, class Value>
    void insertWithUndo(rse::UndoableValue<K>& k, pair<Key, Value> newValue)
    {
        if (k.v.contains(newValue.first)) {
            return;
        }
        auto undoFn = [this, kInUndo = &k, newValueFirst = newValue.first]() mutable {
            kInUndo->v.erase(newValueFirst);
            kInUndo->setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        };
        auto redoFn = [this, kInRedo = &k, newValueInRedo = MOVE(newValue)]() mutable {
            kInRedo->v.insert(newValueInRedo);
            kInRedo->setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        };
        undoableOpReceived(MOVE(undoFn), MOVE(redoFn));
    }
    // Assume K is a container with push_back.
    template<class K, class Value>
    void pushBackWithUndo(rse::UndoableValue<K>& k, Value newValue)
    {
        auto undoFn = [this, kInUndo = &k]() mutable {
            kInUndo->v.pop_back();
            kInUndo->setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        };
        auto redoFn = [this, kInRedo = &k, newValueInRedo = MOVE(newValue)]() mutable {
            kInRedo->v.push_back(newValueInRedo);
            kInRedo->setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        };
        undoableOpReceived(MOVE(undoFn), MOVE(redoFn));
    }
    template<class K, class Value>
    void setWithUndo(rse::UndoableValue<K>& k, Value newValue)
    {
        if (newValue == k.v) {
            return;
        }
        auto undoFn = [this, kInUndo = &k, oldValue = k.v]() mutable {
            kInUndo->v = oldValue;
            kInUndo->setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        };
        auto redoFn = [this, kInRedo = &k, newValueInRedo = MOVE(newValue)]() mutable {
            kInRedo->v = newValueInRedo;
            kInRedo->setTimestampAndMarkDownstreamNodesOutOfDate(nextTimestamp++);
        };
        undoableOpReceived(MOVE(undoFn), MOVE(redoFn));
    }

    rse::ScopedUndoables beginUndoables();

private:
    friend class rse::ScopedUndoables;

    optional<vector<rse::UpstreamNode>> inputCollectorDuringRegistration;
    uint64_t nextTimestamp = 1; // timestamp = 0 means uninitialized
    struct UndoRedoNodeBase {
        // Not sure if it'll ever matter but execute undoOps in reverse order.
        vector<pair<function<void()>, function<void()>>> undoRedoOps;

        UndoRedoNodeBase() = default;
        UndoRedoNodeBase(function<void()> undoFn, function<void()> redoFn);

        void pushBack(function<void()> undoFn, function<void()> redoFn);

        void executeUndo();
        void executeRedo();
    };
    struct UndoRedoNode : public UndoRedoNodeBase {
        chr::system_clock::time_point systemTimePoint;
        chr::steady_clock::time_point steadyTimePoint;
        explicit UndoRedoNode(UndoRedoNodeBase base);
        UndoRedoNode(function<void()> undoFn, function<void()> redoFn);
    };
    optional<UndoRedoNodeBase> undoablesCollector;
    deque<UndoRedoNode> undoRedoHistory;
    deque<UndoRedoNode>::iterator nextNodeToRedo = undoRedoHistory.end();

    void addToInputCollectorIfNeeded(const rse::NodeBase& nb);
    bool updateIfNeededCore(const rse::ComputedNodeBase& cnb);
    bool updateIfNeededCore2(const rse::ComputedNodeBase& cnb);

    void registerUpdaterCore_prepare(rse::ComputedNodeBase& cnb);
    void registerUpdaterCore_finalize(rse::ComputedNodeBase& cnb);

    void finishUndoables();

    void undoableOpReceived(function<void()> undoFn, function<void()> redoFn);

    template<class V>
    void registerUpdaterCore(
      rse::Computed<V>& k, function<V()> computeFnArg, optional<initializer_list<rse::UpstreamNode>> upstreamNodes
    )
    {
        registerUpdaterCore_prepare(k);

        if (upstreamNodes) {
            inputCollectorDuringRegistration.value().assign(*upstreamNodes);
        } else {
            computeFnArg();
        }
        k.computeAndUpdateIfDifferentFn = [&k, computeFn = MOVE(computeFnArg)]() -> bool {
            auto newValue = computeFn();
            if (newValue == k.v) {
                return false;
            }
            k.v = MOVE(newValue);
            return true;
        };

        registerUpdaterCore_finalize(k);
    }
};
