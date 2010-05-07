#include "InputManager.h"

namespace
{
	uint32 gKeysPressed = 0;
	uint32 gRawKeysPressed = 0;
	uint32 gKeysHeld = 0;
	
	const uint32 PauseKey = KEY_START;
	const uint32 UnpauseOneFrameKey = KEY_R;
	bool gIsPaused = false;
	bool gUnpauseOneFrame = false;
}

namespace InputManager
{
	void Update()
	{
		//@NOTE: scanKeys() and keysDown() must only be called once per frame, never more

		scanKeys();

		gKeysPressed = gRawKeysPressed = keysDown();
		gKeysHeld = keysHeld();

		if (gKeysPressed & PauseKey)
		{
			gIsPaused = !gIsPaused;
		}

		gUnpauseOneFrame = false;
		if (gKeysPressed & UnpauseOneFrameKey)
		{
			gUnpauseOneFrame = true;
		}
		
		// Pause key only queryable via IsPaused()
		gKeysPressed &= (~PauseKey & ~UnpauseOneFrameKey);

		// When paused, for debugging purposes, it's useful to consider
		// held keys as pressed keys
		if (gIsPaused)
		{
			gKeysPressed |= gKeysHeld;
		}
	}

	bool IsPaused()
	{
		return gIsPaused && !gUnpauseOneFrame;
	}

	uint32 GetKeysPressed()
	{
		return gKeysPressed;
	}

	uint32 GetRawKeysPressed()
	{
		return gRawKeysPressed;
	}

	uint32 GetKeysHeld()
	{
		return gKeysHeld;
	}

	void ClearKeysPressed(uint32 keysToClear)
	{
		gKeysPressed &= ~keysToClear;
	}

	void ClearKeysHeld(uint32 keysToClear)
	{
		gKeysHeld &= ~keysToClear;
	}
}
