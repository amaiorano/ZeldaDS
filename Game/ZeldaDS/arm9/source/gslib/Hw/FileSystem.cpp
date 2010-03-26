#include "FileSystem.h"

// #include <nds.h>
#include <nds/interrupts.h>
#include <filesystem.h>
#include <stdio.h>

namespace FileSystem
{
	void Init()
	{
		if ( !nitroFSInit() )
		{
			printf(
				"Failed to init file system. You might need to DLDI patch this rom, or "
				"run with the Homebrew Menu. If using an emulator, trying running as a "
				"GBA cart."
				);

			// Infinite loop
			while (true)
			{
				swiWaitForVBlank();
			}
		}
	}
}
