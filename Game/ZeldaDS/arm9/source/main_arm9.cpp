#include "gslib/Core/Core.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Hw/ExceptionHandler.h"
#include "gslib/Hw/FileSystem.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/Sprite.h"
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
#include "gslib/Game/GameHelpers.h"

#include "data/overworld_bg.h"
#include "data/overworld_fg.h"
#include "data/characters.h"
#include "data/items.h"

//@TODO: MEMORY MANAGEMENT:
// 4 MB ram on DS
// Amount left for heap: 4 MB - 32k (internal libnds stuff) - <your program size>
// <your program size> can be found using arm-eabi-size.exe <elf_file>

Player* gpPlayer = 0;
Enemy* gpEnemies[5] = {0};

struct GameStates
{
	struct GameSharedStateData : SharedStateData
	{
	};

	typedef ClientStateBase<GameSharedStateData> GameStateBase;

	struct Root : GameStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<InitGame>();
		}
	};

	struct InitGame : GameStateBase
	{
		virtual void OnEnter()
		{
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
			return SiblingTransition<LoadMap>();
		}
	};

	struct LoadMap : GameStateBase
	{
		virtual void OnEnter()
		{
			WorldMap& worldMap = WorldMap::Instance();
			worldMap.Init(20, 10);
			//worldMap.TEMP_LoadRandomMap();
			worldMap.LoadMap("Maps/TestMap.map");
			SceneGraph::Instance().SetWorldMap(worldMap);

			Vector2I startScreen(0, 0);
			ScrollingMgr::Instance().Init(startScreen);

			// Once map is loaded, position the player (normally the map should tell us where)
			gpPlayer = new Player();
			Vector2I initPos(MathEx::Rand(HwScreenSizeX - 16), MathEx::Rand(HwScreenSizeY - 16));
			initPos = Camera::Instance().ScreenToWorld(initPos);
			gpPlayer->Init(initPos);
			SceneGraph::Instance().AddNode(*gpPlayer);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<NormalPlay>();
		}
	};

	struct NormalPlay : GameStateBase
	{
		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			TEMP_RespawnEnemiesIfAllDead();			

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
		void TEMP_RespawnEnemiesIfAllDead()
		{
			if (ScrollingMgr::Instance().IsScrolling())
				return;

			const bool bAllEnemiesDead = SceneGraph::Instance().GetEnemyList().size() == 0;
			if (bAllEnemiesDead)
			{
				printf("All enemies dead! Respawning...\n");

				for (uint16 i=0; i<NUM_ARRAY_ELEMS(gpEnemies); ++i)
				{
					delete gpEnemies[i];

					// Compute a random world position for the enemy
					Vector2I initPos(MathEx::Rand(HwScreenSizeX - 16), MathEx::Rand(HwScreenSizeY - 16));
					initPos = Camera::Instance().ScreenToWorld(initPos);

					gpEnemies[i] = new Enemy();
					gpEnemies[i]->Init(initPos);
					SceneGraph::Instance().AddNode(*gpEnemies[i]);
				}
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
int main(void) 
{
	ExceptionHandler::EnableDefaultHandler();
	GraphicsEngine::Init(GameMetaTileSizeX, GameMetaTileSizeY);
	FileSystem::Init();

	LoadAllGameAnimAssets();

	StateMachine gameStateMachine;
	//gameStateMachine.SetDebugLevel(1);
	gameStateMachine.SetSharedStateData(new GameStates::GameSharedStateData());
	gameStateMachine.SetInitialState<GameStates::Root>();

	while (true)
	{
		InputManager::Update();

		if ( !InputManager::IsPaused() )
		{
			// Logic update

			// NDS refresh rate is 60 Hz, so as long as we don't take too long in one frame, we can just pass in 1 (frame)
			const GameTimeType deltaTime = 1;

			gameStateMachine.Update(deltaTime);
		}

		GraphicsEngine::PreVBlankUpdate();
		GraphicsEngine::WaitForVBlank();
		GraphicsEngine::PostVBlankUpdate();
	}

	return 0;
}