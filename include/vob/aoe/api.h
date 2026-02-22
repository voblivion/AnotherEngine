#pragma once

#include <vob/misc/std/library_macros.h>

#if defined(VOB_AOE_EXPORTS)
#	define VOB_AOE_API VOB_MISTD_LIB_EXPORT
#elif defined(VOB_AOE_IMPORTS)
#	define VOB_AOE_API VOB_MISTD_LIB_IMPORT
#else
#   define VOB_AOE_API
#endif
