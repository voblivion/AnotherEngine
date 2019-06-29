#pragma once
#include <aoe/core/standard/DebugBreak.h>

#define ignorableAssert(cond) if (!(cond)) debugBreak();