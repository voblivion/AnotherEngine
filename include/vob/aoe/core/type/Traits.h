#pragma once

#include <type_traits>

#define enforce(cond) std::enable_if_t<cond>* = nullptr
