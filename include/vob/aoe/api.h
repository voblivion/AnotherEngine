#pragma once

#include <vob/sta/library.h>

#if defined(VOB_AOE_EXPORTS)
#	define VOB_AOE_API VOB_STA_LIB_EXPORT
#else
#	define VOB_AOE_API VOB_STA_LIB_IMPORT
#endif