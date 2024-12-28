#pragma once

#include <cstdint>

template<class T>
struct Id {
    using type = T;
    uint64_t v = 0;

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
