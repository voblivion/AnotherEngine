#pragma once

#if defined(_WIN32)
    // Windows
    #define AOE_WINDOWS

#elif defined(__APPLE__) && defined(__MACH__)

    #include "TargetConditionals.h"

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR

        // iOS
        #define AOE_IOS

    #elif TARGET_OS_MAC

        // MacOS
        #define AOE_MACOS

    #else

        // Unsupported Apple system
        #error This Apple operating system is not supported by AOE library

    #endif

#elif defined(__unix__)

    // UNIX system, see which one it is
    #if defined(__ANDROID__)

        // Android
        #define AOE_ANDROID

    #elif defined(__linux__)

         // Linux
        #define AOE_LINUX

    #elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)

        // FreeBSD
        #define AOE_FREEBSD

    #elif defined(__OpenBSD__)

        // OpenBSD
        #define AOE_OPENBSD

    #else

        // Unsupported UNIX system
        #error This UNIX operating system is not supported by AOE library

    #endif

#else

    // Unsupported system
    #error This operating system is not supported by AOE library

#endif

#if defined(AOE_WINDOWS)

	#define AOE_API_EXPORT __declspec(dllexport)
	#define AOE_API_IMPORT __declspec(dllimport)
	#define AOE_DEPRECATED __declspec(deprecated)
	#ifdef _MSC_VER
		#pragma warning(disable: 4251)
	#endif

#elif __GNUC__ >= 4

	#define AOE_API_EXPORT __attribute__ ((__visibility__ ("default")))
	#define AOE_API_IMPORT __attribute__ ((__visibility__ ("default")))
	#define AOE_DEPRECATED __attribute__ ((deprecated))

#else

	#define AOE_API_EXPORT
	#define AOE_API_IMPORT

#endif

#if defined(AOE_CORE_EXPORTS)

	#define AOE_CORE_API AOE_API_EXPORT

#else

	#define AOE_CORE_API AOE_API_IMPORT

#endif