#include "Id.h"

#include "common.h"

#include "boost/uuid/time_generator_v7.hpp"

uint64_t getNextUniqueU64()
{
    static boost::uuids::time_generator_v7 g;
    static auto expectedThreadId = this_thread::get_id();
    CHECK(this_thread::get_id() == expectedThreadId);
    const auto uuid = g();
    pair<uint64_t, uint64_t> u64pair;
    static_assert(sizeof(u64pair) == 16);
    std::copy(uuid.begin(), uuid.end(), reinterpret_cast<uint8_t*>(&u64pair));
    return u64pair.first ^ u64pair.second ^ std::hash<std::thread::id>{}(expectedThreadId);
}
