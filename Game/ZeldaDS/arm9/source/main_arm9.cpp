#include "gslib/Core/Core.h"

#include "gslib/Hw/ExceptionHandler.h"
#include "gslib/Hw/CpuClock.h"
#include "gslib/Hw/FileSystem.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/AudioEngine.h"

#include "gslib/Game/DebugVars.h"
#include "gslib/Game/WorldMap.h"
#include "gslib/Game/GameFlowMgr.h"

//@TODO: MEMORY MANAGEMENT:
// 4 MB ram on DS
// Amount left for heap: 4 MB - 32k (internal libnds stuff) - <your program size>
// <your program size> can be found using arm-eabi-size.exe <elf_file>

//---------------------------------------------------------------------------------
#if DEBUG_VARS_ENABLED
void UpdateDebugVars()
{
	const uint32& keysHeld = InputManager::GetKeysHeld();
	const uint32& keysPressed = InputManager::GetRawKeysPressed();

	// Print usage to stdout
	if (keysPressed & KEY_SELECT)
	{
		printf("=== Debug Keys ===\n");
		printf("R : Advance one frame (paused)\n");
		printf("L + A : DrawCollisionBounds\n");
	}

	// Debug combo: L + button
	if (keysHeld & KEY_L)
	{
		DEBUG_VAR_TOGGLE_IF(keysPressed & KEY_A, DrawCollisionBounds);
	}
}
#endif // DEBUG_VARS_ENABLED

//---------------------------------------------------------------------------------
int main(void) 
{
	ExceptionHandler::EnableDefaultHandler();
	GraphicsEngine::Init(GameMetaTileSizeX, GameMetaTileSizeY);
	FileSystem::Init();

	GameFlowMgr& gameFlowMgr = GameFlowMgr::Instance();
	gameFlowMgr.Init();
	gameFlowMgr.SetTargetWorldMap("Maps/TestMap.map", UseMapSpawnPosition); // Initial map

	bool pausedLastFrame = false;

	while (true)
	{
		CpuClock::Update();
		InputManager::Update();

		const bool pausedThisFrame = InputManager::IsPaused();

		// Handle pause transitions
		//@TODO: Hook up callbacks for OnPaused/OnResumed, this is confusing
		if ( !pausedThisFrame )
		{
			if (pausedLastFrame)
			{
				pausedLastFrame = false;
				AudioEngine::SetPaused(false);
			}
		}
		else if (!pausedLastFrame)
		{
			pausedLastFrame = true;
			AudioEngine::SetPaused(true);
		}

#if DEBUG_VARS_ENABLED
		UpdateDebugVars();
#endif

		// NDS refresh rate is 60 Hz, so as long as we don't take too long in one frame, we can just pass in 1 (frame)
		GameTimeType deltaTime = pausedThisFrame? 0 : 1;

		gameFlowMgr.Update(deltaTime);

		GraphicsEngine::Update(deltaTime);
		GraphicsEngine::Render(deltaTime);

		DBG_STATEMENT(++gDebugFrameCount);
	}

	return 0;
}
