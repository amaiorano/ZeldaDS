#include "ExceptionHandler.h"
#include <nds/ndstypes.h>
#include <nds/arm9/exceptions.h> //@TODO: submit patch with the extern "C" change I made to this header

namespace
{
	void QuietExceptionHandler()
	{
		while (true) { }
	}
}

namespace ExceptionHandler
{
	void EnableDefaultHandler()
	{
		// Use this to get code line from address:
		// c:\DATA\Programming\Projects\NOW\DS\devkitPro\devkitARM\bin>arm-eabi-addr2line.exe -i -C -f -e "..\..\..\Projects\ZeldaDS\ZeldaDS\arm9\ZeldaDS.arm9.elf" 02002897
		defaultExceptionHandler();
	}
	
	void EnableQuietHandler()
	{
		setExceptionHandler(QuietExceptionHandler);
	}
}

