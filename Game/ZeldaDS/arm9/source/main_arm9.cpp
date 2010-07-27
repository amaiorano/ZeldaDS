#include "gslib/Core/Core.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Hw/ExceptionHandler.h"
#include "gslib/Hw/FileSystem.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Hw/CpuClock.h"
#include "gslib/Hw/AudioEngine.h"
#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Anim/AnimControl.h"

#include "gslib/Game/ScrollingMgr.h"
#include "gslib/Game/Camera.h"
#include "gslib/Game/WorldMap.h"
#include "gslib/Game/GameAnims.h"
#include "gslib/Game/SceneGraph.h"
#include "gslib/Game/PhysicsSimulator.h"
#include "gslib/Game/Player.h"
#include "gslib/Game/Enemy.h"
#include "gslib/Game/Weapon.h"
#include "gslib/Game/EnemyFactory.h"
#include "gslib/Game/GameHelpers.h"
#include "gslib/Game/DebugVars.h"

#include "data/overworld_bg.h"
#include "data/overworld_fg.h"
#include "data/characters.h"
#include "data/soundbank.h"

//@TODO: MEMORY MANAGEMENT:
// 4 MB ram on DS
// Amount left for heap: 4 MB - 32k (internal libnds stuff) - <your program size>
// <your program size> can be found using arm-eabi-size.exe <elf_file>

struct GameStates
{
	struct GameSharedStateData : SharedStateData
	{
	};

	typedef StateT<GameSharedStateData> GameState;

	struct Root : GameState
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<InitGame>();
		}
	};

	struct InitGame : GameState
	{
		virtual void OnEnter()
		{
			LoadAllGameAnimAssets();

			// Palettes for both bg2 and bg3 should be the same...
			GraphicsEngine::LoadBgPalette(overworld_bgPal, sizeof(overworld_bgPal)); //@TODO: look into grit palette sharing
			
			// Load sprite palette
			GraphicsEngine::LoadSpritePalette(charactersPal, sizeof(charactersPal));

			GraphicsEngine::SetBgFontColor( RGB8(255, 255, 255) );
			GraphicsEngine::SetSubBgFontColor( RGB8(255, 255, 0) );
			
			GraphicsEngine::GetBgLayer(2).LoadTilesImage(overworld_fgTiles, sizeof(overworld_fgTiles));
			GraphicsEngine::GetBgLayer(3).LoadTilesImage(overworld_bgTiles, sizeof(overworld_bgTiles));

			//GraphicsEngine::GetBgLayer(0).ActivateTextLayer();
			//printf("Testing text on bg layer 0!\n\");
		}
		
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<EnableRendering>();
		}
	};

	struct EnableRendering : GameState
	{
		uint16 mNumElapsedFrames;

		EnableRendering() : mNumElapsedFrames(0)
		{
			// Before enabling backgrounds, start faded out
			GraphicsEngine::FadeScreen(FadeScreenDir::Out, 0);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			// Wait one frame for the faded out black screen to render once
			// before showing the backgrounds (sucks that we have to do this)
			if (mNumElapsedFrames >= 1)
			{
				GraphicsEngine::SetAllBgsEnabled(true);
				return SiblingTransition<LoadMap>();
			}

			++mNumElapsedFrames;
			return NoTransition();
		}
	};

	struct LoadMap : GameState
	{
		virtual void OnEnter()
		{
			WorldMap& worldMap = WorldMap::Instance();
			worldMap.Init(20, 10);
			worldMap.LoadMap("Maps/TestMap.map");
			SceneGraph::Instance().SetWorldMap(worldMap);

			AudioEngine::LoadBank("Audio/soundbank.bin");
			AudioEngine::PlayMusic(MOD_OVERWORLD3);

			const WorldMap::PlayerSpawnData& playerSpawnData = worldMap.GetPlayerSpawnData();

			ScrollingMgr::Instance().Init(playerSpawnData.mScreen);

			Player* pPlayer = new Player();
			Vector2I initPos(playerSpawnData.mPos);
			pPlayer->Init(initPos);
			SceneGraph::Instance().AddNode(pPlayer);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<PlayingMap>();
		}
	};

	struct PlayingMap : GameState, IScrollingEventListener
	{
		bool mSpawnEnemiesNextFrame;

		virtual void OnEnter()
		{
			// Normally set by ScrollingMgr callbacks, but we want to spawn enemies for
			// initial screen
			mSpawnEnemiesNextFrame = true;

			// Fade in screen
			GraphicsEngine::FadeScreen(FadeScreenDir::In, SEC_TO_FRAMES(0.5f));

			ScrollingMgr::Instance().AddEventListener(this);
		}

		virtual void OnExit()
		{
			ScrollingMgr::Instance().RemoveEventListener(this);
			RemoveNodesBeforeChangingScreens();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			// Spawn enemies before sim + render
			if (mSpawnEnemiesNextFrame)
			{
				SpawnEnemiesForCurrentScreen();
				mSpawnEnemiesNextFrame = false;
			}

			// Sim
			SceneGraph::Instance().Update(deltaTime);
			WorldMap::Instance().Update(deltaTime);

			// Physics
			PhysicsSimulator::Instance().IntegrateAndApplyCollisions(deltaTime);

			DEBUG_CheckForQuickScroll();

			// Scrolling
			ScrollingMgr::Instance().Update(deltaTime);

			// Render
			SceneGraph::Instance().Render(deltaTime);
		}

	private:
		virtual void OnScrollingBegin()
		{
			RemoveNodesBeforeChangingScreens();
		}

		virtual void OnScrollingEnd()
		{
			mSpawnEnemiesNextFrame = true;
		}

		void RemoveNodesBeforeChangingScreens()
		{
			// Remove all enemies, weapons, etc.
			WeaponList& playerWeapons = SceneGraph::Instance().GetPlayerWeaponList();
			for (WeaponList::iterator iter = playerWeapons.begin(); iter != playerWeapons.end(); ++iter)
			{
				SceneGraph::Instance().RemoveNodePostUpdate(*iter);
			}
			WeaponList& enemyWeapons = SceneGraph::Instance().GetEnemyWeaponList();
			for (WeaponList::iterator iter = enemyWeapons.begin(); iter != enemyWeapons.end(); ++iter)
			{
				SceneGraph::Instance().RemoveNodePostUpdate(*iter);
			}
			EnemyList& enemies = SceneGraph::Instance().GetEnemyList();
			for (EnemyList::iterator iter = enemies.begin(); iter != enemies.end(); ++iter)
			{
				Enemy* pEnemy = *iter;
				SceneGraph::Instance().RemoveNodePostUpdate(pEnemy);
			}
		}

		void SpawnEnemiesForCurrentScreen()
		{
			ASSERT(SceneGraph::Instance().GetSceneNodeList().size()== 1); // Only player should exist

			WorldMap::SpawnDataList spawnDataList;
			WorldMap::Instance().GetSpawnDataForScreen(ScrollingMgr::Instance().GetCurrScreen(), spawnDataList);

			WorldMap::SpawnDataList::iterator iter = spawnDataList.begin();
			for ( ; iter != spawnDataList.end(); ++iter)
			{
				WorldMap::SpawnData& spawnData = *iter;
				Enemy* pEnemy = EnemyFactory::CreateEnemy(spawnData.mGameActor);
				pEnemy->Init(spawnData.mPos);
				SceneGraph::Instance().AddNode(pEnemy);
			}
		}

		void DEBUG_CheckForQuickScroll()
		{
			const uint32& keysHeld = InputManager::GetKeysHeld();
			const uint32& keysPressed = InputManager::GetKeysPressed();
			WorldMap& worldMap = WorldMap::Instance();

			ScrollDir::Type scrollDir = ScrollDir::None;

			const Vector2I& currScreen = ScrollingMgr::Instance().GetCurrScreen();
			if ( keysHeld & KEY_L )
			{
				if ( keysPressed & KEY_LEFT	&& currScreen.x-1 >= 0 )							scrollDir = ScrollDir::Left;
				if ( keysPressed & KEY_RIGHT && currScreen.x+1 < worldMap.GetNumScreensX())		scrollDir = ScrollDir::Right;
				if ( keysPressed & KEY_UP && currScreen.y-1 >= 0 )								scrollDir = ScrollDir::Up;
				if ( keysPressed & KEY_DOWN	&& currScreen.y+1 < worldMap.GetNumScreensY())		scrollDir = ScrollDir::Down;
			}

			ScrollingMgr& scrollingMgr = ScrollingMgr::Instance();
			if (!scrollingMgr.IsScrolling() && scrollDir != ScrollDir::None)
			{
				scrollingMgr.StartScrolling(scrollDir);
			}
		}
	};
};

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

	StateMachine gameStateMachine;
	//gameStateMachine.SetDebugLevel(1);
	gameStateMachine.SetSharedStateData(new GameStates::GameSharedStateData());
	gameStateMachine.SetInitialState<GameStates::Root>();

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

 		gameStateMachine.Update(deltaTime);

		GraphicsEngine::Update(deltaTime);
		GraphicsEngine::Render(deltaTime);

		DBG_STATEMENT(++gDebugFrameCount);
	}

	return 0;
}
