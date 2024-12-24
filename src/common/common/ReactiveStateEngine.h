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
class ReactiveStateEngine
{
public:
    template<class K>
    class BorrowedVariableToSet
    {
    public:
        explicit BorrowedVariableToSet(ReactiveStateEngine* thatArg, K* variableArg, intptr_t offsetArg)
            : that(thatArg)
            , variable(variableArg)
            , offset(offsetArg)
        {
        }
        BorrowedVariableToSet(const BorrowedVariableToSet&) = delete;
        BorrowedVariableToSet(BorrowedVariableToSet&& y)
            : that(y.that)
            , variable(y.variable)
            , offset(y.offset)
        {
            y.variable = nullptr;
        }
        ~BorrowedVariableToSet()
        {
            if (variable) {
                that->markChangedPrivate(offset);
            }
        }
        K& operator*() const
        {
            return *variable;
        }
        K* operator->() const
        {
            return variable;
        }

    private:
        ReactiveStateEngine* that;
        K* variable;
        intptr_t offset;
    };

    template<class T, class Fn>
    void registerUpdater(T& k, Fn&& updaterFnArg)
    {
        registerUpdaterCore(k, function<T()>(std::forward<Fn>(updaterFnArg)));
    }

    // Return true if it had to be updated (wasn't up-to-date).
    template<class T>
    bool updateIfNeeded(const T& k)
    {
        return updateIfNeededCore(getOffset(k));
    }

    // Query the value of k.
    //
    // If it is called withing a registered update function, the variableBeingUpdatedStack won't be empty
    // and a dependency relationship will be established between `variableBeingUpdatedStack.back()` and `k`.
    template<class T>
    const T& get(const T& k)
    {
        updateIfNeededCore(getOffset(k));
        return k;
    }

    // Assign new value to the variable, and
    // - if this is the initial `set` for the variable, or
    // - the new value is different from the current one
    // then recursively mark dependencies as outdated.
    template<class K, class V>
    bool set(K& k, V&& newValue)
    {
        auto offset = getOffset(k);
        auto& v = variables[offset];
        if (v.timestamp > 0 && k == newValue) {
            return false;
        }
        k = std::forward<V>(newValue);
        v.timestamp = nextTimestamp++;
        for (auto d : v.dependents) {
            bumpThisAndDownstreamVariablesMaxUpstreamTimestamp(d, v.timestamp);
        }
        return true;
    }

    // Assign new value to the variable and recursively mark dependencies as outdated.
    // Do not compare new value to the existing value, always assume that it has changed.
    // `setAsDifferent` is a useful alternative to `set` if we don't want to call the equality operator for the type
    // (because it doesn not exist or expensive).
    template<class K, class V>
    void setAsDifferent(K& k, V&& newValue)
    {
        auto offset = getOffset(k);
        auto& v = variables[offset];
        k = std::forward<V>(newValue);
        v.timestamp = nextTimestamp++;
        for (auto d : v.dependents) {
            bumpThisAndDownstreamVariablesMaxUpstreamTimestamp(d, v.timestamp);
        }
    }

    template<class K>
    auto borrowThenSet(K& k)
    {
        return BorrowedVariableToSet<K>(this, &k, getOffset(k));
    }

private:
    bool updateIfNeededCore(intptr_t koffset)
    {
        auto& vk = variables[koffset];

        if (!variableBeingUpdatedStack.empty()) {
            auto qoffset = variableBeingUpdatedStack.back();
            assert(qoffset != koffset);
            if (vk.addDependent(qoffset)) {
                auto& vq = variables[qoffset];
                if (vk.timestamp > vq.maxUpstreamTimestamp) {
                    vq.maxUpstreamTimestamp = vk.timestamp;
                }
            }
        }

        if (vk.maxUpstreamTimestamp <= vk.timestamp && vk.timestamp > 0) {
            return false;
        }

        // Getting the value of an uninitialized (timestamp==0) or stale (maxUpstreamTimestamp > timestamp) variable.

        assert(vk.updateFn);
        vk.updateFn();

        return true;
    }

    template<class T>
    void registerUpdaterCore(T& k, function<T()> updaterFnArg)
    {
        auto offset = getOffset(k);
        auto& v = variables[offset];
        assert(!v.updateFn);
        v.updateFn = [this, &k, updateFn = MOVE(updaterFnArg), offset]() {
            variableBeingUpdatedStack.push_back(offset);
            set(k, updateFn());
            assert(!variableBeingUpdatedStack.empty());
            variableBeingUpdatedStack.pop_back();
        };
    }

    void bumpThisAndDownstreamVariablesMaxUpstreamTimestamp(intptr_t offset, uint64_t timestamp);

    template<class K>
    intptr_t getOffset(const K& k) const
    {
        return reinterpret_cast<intptr_t>(&k);
    }
    void markChangedPrivate(intptr_t offset)
    {
        auto& v = variables[offset];
        v.timestamp = nextTimestamp++;
        for (auto d : v.dependents) {
            bumpThisAndDownstreamVariablesMaxUpstreamTimestamp(d, v.timestamp);
        }
    }

    struct Variable {
        uint64_t maxUpstreamTimestamp = 0;
        uint64_t timestamp = 0;
        // Dependents are other variables that should be updated when this is changing.
        vector<intptr_t> dependents;
        function<void()> updateFn;

        bool addDependent(intptr_t d);
    };

    uint64_t nextTimestamp = 1;
    unordered_map<intptr_t, Variable> variables;
    vector<intptr_t> variableBeingUpdatedStack;
};
