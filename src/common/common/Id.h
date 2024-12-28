#pragma once

#include <cstddef>
#include <cstdint>
#include <vector> // for std::hash

// Creation is single-threaded (main thread).

uint64_t getNextUniqueU64();

template<class T>
struct Id {
    inline static Id make()
    {
        return Id(getNextUniqueU64());
    }

    using type = T;

    uint64_t v = 0;

    Id() = delete;
    explicit Id(uint64_t id)
        : v(id)
    {
    }
    bool operator==(const Id&) const = default;
};

template<class T>
struct std::hash<Id<T>> {
    std::size_t operator()(const Id<T>& x) const noexcept
    {
        return std::hash<uint64_t>{}(x.v);
    }
};
