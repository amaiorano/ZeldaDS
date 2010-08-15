#ifndef CORE_CORE_H
#define CORE_CORE_H

#include "Config.h"
#include "Types.h"
#include "Assert.h"

//@TODO: Remove in release?
#include <cstdio>
#include <malloc.h>

typedef uint16 GameTimeType; // Elapsed frames
const uint16 TargetFramesPerSecond = 60;

// Convert seconds to frames; i.e. if you want an anim frame to last 2 seconds, it will last 2 * 60 = 120 frames
#define SEC_TO_FRAMES(seconds)	static_cast<GameTimeType>(seconds * TargetFramesPerSecond)

#if PRINT_TO_EMULATOR_CONSOLE
	// Prints to console within certain emulators (iDeaS and deSmuME)
	extern void EmuConsolePrintF(const char* format, ...);

	// Remap all printf calls (requires that Core.h be included before any printf calls)
	#undef printf
	#define printf EmuConsolePrintF
#endif


#define PRINT_VECTOR2I(vec) printf("%s: (%d, %d)\n", #vec, vec.x, vec.y)

// Number of c-style array elements
#define NUM_ARRAY_ELEMS(arr) (sizeof(arr)/sizeof(arr[0]))

// Global frame count for debugging
DBG_STATEMENT(extern uint32 gDebugFrameCount);

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
