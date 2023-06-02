#pragma once

#include <vob/misc/std/library_macros.h>

#if defined(VOB_AOE_EXPORTS)
#	define VOB_AOE_API VOB_MISTD_LIB_EXPORT
#else
#	define VOB_AOE_API VOB_MISTD_LIB_IMPORT
#endif
