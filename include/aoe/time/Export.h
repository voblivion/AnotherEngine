#pragma once

#include <aoe/Config.h>

#if defined(AOE_TIME_EXPORTS)

	#define AOE_TIME_API AOE_API_EXPORT

#else

	#define AOE_TIME_API AOE_API_IMPORT

#endif