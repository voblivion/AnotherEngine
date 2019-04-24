#pragma once

#include <aoe/Config.h>

#if defined(AOE_COMMON_EXPORTS)

	#define AOE_COMMON_API AOE_API_EXPORT

#else

	#define AOE_COMMON_API AOE_API_IMPORT

#endif