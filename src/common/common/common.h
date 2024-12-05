#pragma once

#include "std.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "fmt/format.h"

#define MOVE(X) std::move(X)

inline void __inline_void_function_with_empty_body__() {}
#define NOP __inline_void_function_with_empty_body__()

#define UNUSED [[maybe_unused]]
