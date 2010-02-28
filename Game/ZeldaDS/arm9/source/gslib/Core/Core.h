#ifndef CORE_CORE_H
#define CORE_CORE_H

#include "Types.h"
#include <nds/arm9/sassert.h>

//@TODO: Remove in non-final?
#include <cstdio>
#include <malloc.h>

#define PLATFORM_NDS

typedef uint16 GameTimeType; // Elapsed frames
const uint16 TargetFramesPerSecond = 60;

// Convert seconds to frames; i.e. if you want an anim frame to last 2 seconds, it will last 2 * 60 = 120 frames
#define SEC_TO_FRAMES(seconds)	static_cast<GameTimeType>(seconds * TargetFramesPerSecond)

// Disable this when compiling for actual hardware (otherwise printf calls will crash)
#define PRINT_TO_EMULATOR_CONSOLE 1
#if PRINT_TO_EMULATOR_CONSOLE == 1
	// Prints to console within certain emulators (iDeaS and deSmuME)
	extern void EmuConsolePrintF(const char* format, ...);

	// Remap all printf calls (requires that Core.h be included before any printf calls)
	#undef printf
	#define printf EmuConsolePrintF
#endif


#define PRINT_VECTOR2I(vec) printf("%s: (%d, %d)\n", #vec, vec.x, vec.y)

// Number of c-style array elements
#define NUM_ARRAY_ELEMS(arr) (sizeof(arr)/sizeof(arr[0]))

// Assert functions
#define ASSERT(exp)				sassert(exp, "")
#define ASSERT_MSG(exp, msg)	sassert(exp, msg)
#define FAIL()					sassert(false, "FAIL")
#define FAIL_MSG(msg)			sassert(false, "FAIL: " msg)

// Compile-time assertion (@TODO: Should this be moved to Generic.h?)
#define CT_ASSERT(expr)				struct __UNIQUE_NAME { unsigned int no_message : expr; }
#define CT_ASSERT_MSG(expr, msg)	struct __UNIQUE_NAME { unsigned int msg : expr; }
#define __UNIQUE_NAME				__MAKE_NAME(__LINE__)
#define __MAKE_NAME(line)			__MAKE_NAME2(line)
#define __MAKE_NAME2(line)			ct_assert_failure_ ## line

// Useful for wrapping debug-only statements; i.e. declaring variables used in assertions
#ifdef NDEBUG
#define DBG_STATEMENT(s)
#else
#define DBG_STATEMENT(s) s
#endif

// Memory debugging
extern struct mallinfo mi_start;
extern struct mallinfo mi_end;
extern int mi_bytes_delta;

#define HEAP_CHECK_SIZE() mi_start = mallinfo()

#define HEAP_ASSERT_SAME_SIZE(label) \
	mi_end = mallinfo(); \
	mi_bytes_delta = mi_end.uordblks - mi_start.uordblks; \
	ASSERT_MSG(mi_bytes_delta == 0, label ": memory leak!")

#define HEAP_REPORT_DELTA_SIZE(label) \
	mi_end = mallinfo(); \
	mi_bytes_delta = mi_end.uordblks - mi_start.uordblks; \
	printf(label ": heap delta: %d\n", mi_bytes_delta)


// STL macros
#define REMOVE_FROM_CONTAINER(ContainerType, container, elem) \
	{ \
		ContainerType::iterator iter = std::find(container.begin(), container.end(), elem); \
		ASSERT_MSG(iter != container.end(), "Removing node " #elem " not in list " #container); \
		container.erase(iter); \
	}

#endif // CORE_CORE_H
