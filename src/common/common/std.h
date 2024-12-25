#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <expected>
#include <functional>
#include <initializer_list>
#include <memory>
#include <numbers>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>

namespace chr = std::chrono;
namespace this_thread = std::this_thread;
namespace vi = std::ranges::views;
namespace ra = std::ranges;

using std::array;
using std::deque;
using std::expected;
using std::function;
using std::initializer_list;
using std::make_unique;
using std::monostate;
using std::nullopt;
using std::optional;
using std::pair;
using std::span;
using std::string;
using std::string_view;
using std::unexpected;
using std::unique_ptr;
using std::unordered_map;
using std::variant;
using std::vector;
