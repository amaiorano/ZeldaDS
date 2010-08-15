#include "Assert.h"
#include <string.h>

#if ASSERTS_ENABLED

#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include <nds/arm9/console.h>
#include <nds/ndstypes.h> //@NOTE: required include before <nds/arm9/input.h> (should let libnds authors know)
#include <nds/arm9/input.h> // Include here for key constants
#include <stdio.h>
#include <stdarg.h>

namespace AssertImpl
{
	void AssertFunctor::Printf(const char* format, ...)
	{
		static char msg[1024];
		msg[0] = '\0';
		va_list args;
		va_start(args, format);
		vsprintf(msg, format, args);
		
		// If subscreen already in text mode, don't need to switch it (will keep whatever output was there)
		if (!GraphicsEngine::IsInitialized() || !GraphicsEngine::GetSubBgLayer(0).IsTextLayer())
		{
			consoleDemoInit();
		}
		else
		{
			//@TODO: User palette may not match the default one, so if we want to make sure the colors are right,
			// we should somehow load the default palette here (save the current one, load default, reload current
			// before exiting function)
		}

		const bool hasMsg = strlen(msg) > 0;

		//@NOTE: VT-100 color codes:
		// 30 black
		// 31 red
		// 32 green
		// 33 yellow
		// 34 blue
		// 35 magenta
		// 36 cyan
		// 37 white
		// 38 unused
		// 39 default (white, palette entry 255)
		// - Add 10 for intensity (i.e. 45 is more intense magenta)
		// - These colors depend on the default color palette layout

		iprintf(
			"\x1b[41m***** ASSERT *****\n"
			"\x1b[47mFile: %s (%d)\n"
			"\x1b[43mCond: %s\n"
			"\x1b[45mMsg : %s\n"
			, mFile, mLine, mCondition, hasMsg? msg : "(none)");

		iprintf("\x1b[32m[A : Continue]\n");
		iprintf("\x1b[32m[X : Always Skip]\n");
		iprintf("\x1b[39m"); // Back to default

		while (true)
		{
			//@NOTE: Calling these input functions directly may invalidate InputMgr's data for the rest of the frame
			scanKeys();
			uint32 keysPressed = keysDown();

			if (keysPressed & KEY_A)
			{
				return;
			}

			if (keysPressed & KEY_X)
			{
				mAlwaysSkip = true;
				return;
			}
		}
	}
}

#endif // ASSERTS_ENABLED
