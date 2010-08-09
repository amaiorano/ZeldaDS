#ifndef GS_CONFIG_H
#define GS_CONFIG_H

#define PLATFORM_NDS

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
#endif // DEVELOPMENT_MODE

// Define build configuration constants
#if defined(NDEBUG)
	#define CONFIG_RELEASE 1
#else
	#define CONFIG_DEBUG 1	
#endif

#endif // GS_CONFIG_H
