#pragma once

#include "std.h"

#include "absl/log/check.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/std.h"

#define MOVE(X) std::move(X)

// Usage:
//
//     NOP; // You can put a breakpoint here.
//
inline void __inline_void_function_with_empty_body__() {}
#define NOP __inline_void_function_with_empty_body__()

#define UNUSED [[maybe_unused]]

// Usage:
//
//     CHECK_OR_RETURN(result == good);
//
// Which turns into:
//
// -   (Debug): assert(result == good);
// - (Release): if (result != good) return;
//
#define CHECK_OR_RETURN(COND)                      \
    do {                                           \
        if (!(COND)) {                             \
            LOG(DFATAL) << "Check failed: " #COND; \
            return;                                \
        }                                          \
    } while ((false))

// Usage:
//
//     CHECK_OR_RETURN_VAL(result == good, "");
//
// Which turns into:
//
// -   (Debug): assert(result == good);
// - (Release): if (result != good) return "";
//
#define CHECK_OR_RETURN_VAL(COND, VAL)             \
    do {                                           \
        if (!(COND)) {                             \
            LOG(DFATAL) << "Check failed: " #COND; \
            return (VAL);                          \
        }                                          \
    } while ((false))

// Usage:
//
//     namespace MySumType {
//         struct A { int a; };
//         struct B { double b; };
//         using V = std::variant<A, B>; // You must use the name `V`.
//     }
//
//     auto v = MAKE_VARIANT_V(MySumType, A{3});
//
// Which turns into:
//
//     auto v = std::variant<MySumType::V>(MySumType::A{3});
//
#define MAKE_VARIANT_V(NAMESPACE, CONSTRUCTOR_CALL_IN_NAMESPACE) NAMESPACE::V(NAMESPACE::CONSTRUCTOR_CALL_IN_NAMESPACE)

// Return if the integer `x` is between 0 and t.size()
template<class X, class T>
    requires std::integral<X>
bool isValidIndexOfContainer(X x, const T& t)
{
    return std::cmp_less_equal(0, x) && std::cmp_less(x, t.size());
}

// switch_variant from https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename... Ts, typename Variant>
auto switch_variant(Variant&& variant, Ts&&... ts)
{
    return std::visit(overloaded{std::forward<Ts>(ts)...}, std::forward<Variant>(variant));
}

template<class R, class T>
R intCast(T t)
{
    static_assert(std::is_integral_v<R>, "The result type of intCast should be integer.");
    static_assert(std::is_integral_v<T>, "Argument for intCast should be integer.");

    // Return with static cast if static_cast is always safe, lossless.
    if constexpr ((std::is_signed_v<R> == std::is_signed_v<T> && sizeof(R) >= sizeof(T))
                  || (std::is_signed_v<R> && sizeof(R) > sizeof(T))) {
        return static_cast<R>(t);
    } else {
        assert(std::in_range<R>(t));
        return static_cast<R>(t);
    }
}

template<class R, class T>
R clampCast(T t)
{
    static_assert(std::is_integral_v<R>, "The result type of clampCast should be integer.");
    static_assert(std::is_integral_v<T>, "Argument for clampCast should be integer.");
    if (std::in_range<R>(t)) {
        return static_cast<R>(t);
    }
    if constexpr (std::is_signed_v<R>) {
        if constexpr (std::is_signed_v<T>) {
            // signed -> signed
            return t > 0 ? std::numeric_limits<R>::max() : std::numeric_limits<R>::min();
        } else {
            // unsigned -> signed
            return std::numeric_limits<R>::max();
        }
    } else {
        if constexpr (std::is_signed_v<T>) {
            // signed -> unsigned
            return t > 0 ? std::numeric_limits<R>::max() : 0;
        } else {
            // unsigned -> unsigned
            std::numeric_limits<R>::max();
        }
    }
}

template<class R, class T>
R floatCast(T t)
{
    static_assert(std::is_floating_point_v<R>, "The result type of floatCast should be floating point.");
    static_assert(std::is_floating_point_v<T>, "Argument for floatCast should be floating point.");
    return static_cast<R>(t);
}
