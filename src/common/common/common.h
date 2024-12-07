#pragma once

#include "std.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#define MOVE(X) std::move(X)

inline void __inline_void_function_with_empty_body__() {}
#define NOP __inline_void_function_with_empty_body__()

#define UNUSED [[maybe_unused]]

#define CHECK_OR_RETURN_VAL(COND, VAL)             \
    do {                                           \
        if (!(COND)) {                             \
            LOG(DFATAL) << "Check failed: " #COND; \
            return (VAL);                          \
        }                                          \
    } while ((false))

template<class X, class T>
    requires std::integral<X>
bool isValidIndexOfContainer(X x, const T& t)
{
    return std::cmp_less_equal(0, x) && std::cmp_less(x, t.size());
}
