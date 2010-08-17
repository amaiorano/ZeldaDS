#ifndef GS_CONFIG_H
#define GS_CONFIG_H

// Define platform constants
#define PLATFORM_NDS

// Define build configuration constants
#if defined(NDEBUG)
	#define CONFIG_RELEASE 1
#else
	#define CONFIG_DEBUG 1
#endif

// Controls whether debug vars are enabled (disable for final game)
#define DEBUG_VARS_ENABLED 1

// If true, printf() outputs to emulator console
// Disable when compiling for actual hardware (otherwise printf calls will crash)
#define PRINT_TO_EMULATOR_CONSOLE 0

// Controls whether audio is enabled
#define AUDIO_ENABLED 1

// Disable when building for actual hardware
#define DEVELOPMENT_MODE 0

#if DEVELOPMENT_MODE
	#undef PRINT_TO_EMULATOR_CONSOLE
	#define PRINT_TO_EMULATOR_CONSOLE 1

	#undef AUDIO_ENABLED
	#define AUDIO_ENABLED 0

	// Treat release as debug (enables asserts, printfs, etc.)
	#if CONFIG_RELEASE
		#undef CONFIG_RELEASE
		#define CONFIG_DEBUG 1
	#endif
#endif // DEVELOPMENT_MODE

#if CONFIG_DEBUG
	#define ASSERTS_ENABLED 1
#endif

#endif // GS_CONFIG_H
