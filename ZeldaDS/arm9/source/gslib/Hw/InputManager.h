#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <nds/ndstypes.h> //@NOTE: required include before <nds/arm9/input.h> (should let libnds authors know)
#include <nds/arm9/input.h> // Include here for key constants

namespace InputManager
{
	void Update();
	bool IsPaused();
	uint32 GetKeysPressed();
	uint32 GetKeysHeld();
	void ClearKeysPressed(uint32 keysToClear = ~0);
	void ClearKeysHeld(uint32 keysToClear = ~0);
}

#endif // INPUT_MANAGER_H
