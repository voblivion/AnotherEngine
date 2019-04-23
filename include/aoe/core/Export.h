#include <aoe/Config.h>

#if defined(AOE_CORE_EXPORTS)

#define AOE_CORE_API AOE_API_EXPORT

#else

#define AOE_CORE_API AOE_API_IMPORT

#endif