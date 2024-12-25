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
    // TODO: consider a `modify` function instead of `BorrowedVariableToSet`.
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
        registerUpdaterCore(k, function<T()>(std::forward<Fn>(updaterFnArg)), nullopt);
    }

    template<class T, class Fn, class... UpstreamVariables>
    void registerUpdater(T& k, Fn&& updaterFnArg, const UpstreamVariables&... upstreamVariables)
    {
        registerUpdaterCore(
          k, function<T()>(std::forward<Fn>(updaterFnArg)), initializer_list<intptr_t>{getOffset(upstreamVariables)...}
        );
    }

    // Return true if it had to be updated (the value has changed during the update).
    template<class T>
    bool updateIfNeeded(const T& k)
    {
        return updateIfNeededCore(getOffset(k));
    }

    // Query the value of k. Might call the updater function.
    template<class T>
    const T& get(const T& k)
    {
        updateIfNeededCore(getOffset(k));
        return k;
    }

    template<class T>
    bool isUpToDate(const T& k)
    {
        return variables[getOffset(k)].upToDate;
    }

    // Assign new value to the variable, and mark all transitive dependencies outdated if the new value is different
    // from the current one.
    template<class K, class V>
    bool set(K& k, V&& newValue)
    {
        auto offset = getOffset(k);
        auto& v = variables[offset];
        CHECK(!v.updateFn && v.upstreamVariables.empty()); // Variables with updateFn should not be set directly.
        if (newValue == k) {
            return false;
        }
        k = std::forward<V>(newValue);
        v.timestamp = nextTimestamp++;
        for (auto d : v.downstreamVariables) {
            markThisAndTransitiveDependenciesOutOfDate(d);
        }
        return true;
    }

    // Like `set` but do not compare new value to the existing value, always assume that it has changed.
    // `setAsDifferent` is a useful alternative to `set` if we don't want to call the equality operator for the type
    // (because it doesn not exist or expensive).
    template<class K, class V>
    void setAsDifferent(K& k, V&& newValue)
    {
        auto offset = getOffset(k);
        auto& v = variables[offset];
        CHECK(!v.updateFn && v.upstreamVariables.empty()); // Variables with updateFn should not be set directly.
        k = std::forward<V>(newValue);
        v.timestamp = nextTimestamp++;
        for (auto d : v.downstreamVariables) {
            markThisAndTransitiveDependenciesOutOfDate(d);
        }
    }

    template<class K>
    auto borrowThenSet(K& k)
    {
        return BorrowedVariableToSet<K>(this, &k, getOffset(k));
    }

private:
    struct Variable {
        // Once a variable gets an updater, this field is reinitialized to false.
        bool upToDate = true;
        uint64_t timestamp = 1;
        uint64_t upstreamProcessedUntilTimestamp = 0;

        vector<intptr_t> upstreamVariables;   // Variables which this variable's updateFn uses as input.
        vector<intptr_t> downstreamVariables; // Variables dependending on this variable.
        function<bool(Variable&)> updateFn;   // Return true if changed.

        // `offset` can be added only once.
        void addDownstreamVariable(intptr_t offset);
    };
    unordered_map<intptr_t, Variable> variables;
    optional<vector<intptr_t>> inputCollectorDuringRegistration;
    uint64_t nextTimestamp = 1; // timestamp = 0 means uninitialized

    bool updateIfNeededCore(intptr_t koffset);
    bool updateIfNeededCore2(Variable& vk);
    void markThisAndTransitiveDependenciesOutOfDate(intptr_t offset);
    void markChangedPrivate(intptr_t offset);

    Variable& registerUpdaterCore_prepare(intptr_t offset);
    void registerUpdaterCore_finalize(intptr_t offset, Variable& v);

    template<class T>
    void registerUpdaterCore(T& k, function<T()> updaterFnArg, optional<initializer_list<intptr_t>> upstreamVariableIds)
    {
        auto offset = getOffset(k);

        auto& v = registerUpdaterCore_prepare(offset);

        if (upstreamVariableIds) {
            inputCollectorDuringRegistration.value().assign(*upstreamVariableIds);
        } else {
            updaterFnArg();
        }
        // Passing Variable& instead of capturing: allows us to use a hash container which invalidates values on update.
        v.updateFn = [this, &k, updateFn = MOVE(updaterFnArg)](Variable& v2) -> bool {
            auto newValue = updateFn();
            if (newValue == k) {
                return false;
            }
            k = MOVE(newValue);
            v2.timestamp = nextTimestamp++;

            return true;
        };

        registerUpdaterCore_finalize(offset, v);
    }

    template<class K>
    intptr_t getOffset(const K& k) const
    {
        return reinterpret_cast<intptr_t>(&k);
    }
};
