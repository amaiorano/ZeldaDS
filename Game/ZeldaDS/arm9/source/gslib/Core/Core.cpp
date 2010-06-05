#include "Core.h"
#include <stdarg.h>

DBG_STATEMENT(uint32 gDebugFrameCount = 0);

// Globals for memory debugging
struct mallinfo mi_start;
struct mallinfo mi_end;
int mi_bytes_delta;

#if PRINT_TO_EMULATOR_CONSOLE
extern "C" void ideasPrint(const char* str);

#define EMU_PRINTS_EXTRA_NEWLINE 1

void EmuConsolePrintF(const char* format, ...)
{
	const int BUFF_SIZE = 4096;
	static char buff[BUFF_SIZE];

	va_list args;
	va_start(args, format);

#if EMU_PRINTS_EXTRA_NEWLINE
	static int idx = 0;
	idx += vsprintf(&buff[idx], format, args);

	//@NOTE: Buffering up strings until last character is newline, then output
	// without the newline. I do this because desmume's implementation of the
	// ideas console print adds its own newline, unfortunately.
	if (idx > 0 && buff[idx-1] == '\n')
	{
		buff[idx-1] = '\0';
		ideasPrint(buff);
		idx = 0;
	}
#else
	vsprintf(buff, format, args);
#endif

	ideasPrint(buff);
}
#endif
