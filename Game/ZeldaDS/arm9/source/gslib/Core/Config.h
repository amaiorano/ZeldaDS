#ifndef GS_CONFIG_H
#define GS_CONFIG_H

#define PLATFORM_NDS

// If true, printf() outputs to emulator console
// Disable when compiling for actual hardware (otherwise printf calls will crash)
#define PRINT_TO_EMULATOR_CONSOLE 0

// Controls whether debug vars are enabled (disable for final game)
#define DEBUG_VARS_ENABLED 1

#endif // GS_CONFIG_H
