#pragma once

#include <aoe/Config.h>

#if defined(AOE_SPACE_EXPORTS)

	#define AOE_SPACE_API AOE_API_EXPORT

#else

	#define AOE_SPACE_API AOE_API_IMPORT

#endif