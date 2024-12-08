#pragma once

#include "std.h"

#include "absl/log/check.h"
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
