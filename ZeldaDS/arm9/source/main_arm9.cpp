#include "gslib/Core/Core.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Hw/ExceptionHandler.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Anim/AnimControl.h"

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

namespace ScrollDir
{
	enum Type { None, Right, Left, Up, Down };
}

struct GameStates
{
	struct GameSharedStateData : SharedStateData
	{
		GameSharedStateData()
			: mScrollDir(ScrollDir::None)
		{
		}

		//@TODO: Externalize scrolling management
		ScrollDir::Type mScrollDir;
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
			worldMap.Init(6, 6);
			worldMap.TEMP_LoadRandomMap();
			worldMap.SetCurrScreen(Vector2I(2,1));
			worldMap.DrawScreenTiles(worldMap.GetCurrScreen(), Vector2I(0,0));

			SceneGraph::Instance().SetWorldMap(worldMap);

			// Once map is loaded, position the player (normally the map should tell us where)
			gpPlayer = new Player();
			Vector2I initPos(MathEx::Rand(HwScreenSizeX - 16), MathEx::Rand(HwScreenSizeY - 16));
			initPos = worldMap.ScreenToWorld(initPos);
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
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (Data().mScrollDir != ScrollDir::None)
			{
				return SiblingTransition<Scrolling>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			//@TEMP: If no more enemies, respawn them all
			{
				const bool bAllEnemiesDead = SceneGraph::Instance().GetEnemyList().size() == 0;
				if (bAllEnemiesDead)
				{
					printf("All enemies dead! Respawning...\n");

					for (uint16 i=0; i<NUM_ARRAY_ELEMS(gpEnemies); ++i)
					{
						delete gpEnemies[i];

						// Compute a random world position for the enemy
						Vector2I initPos(MathEx::Rand(HwScreenSizeX - 16), MathEx::Rand(HwScreenSizeY - 16));
						initPos = WorldMap::Instance().ScreenToWorld(initPos);

						gpEnemies[i] = new Enemy();
						gpEnemies[i]->Init(initPos);
						SceneGraph::Instance().AddNode(*gpEnemies[i]);
					}
				}
			}

			// Sim
			SceneGraph::Instance().Update(deltaTime);

			// Physics
			PhysicsSimulator::Instance().IntegrateAndApplyCollisions(deltaTime);

			// Render
			SceneGraph::Instance().Render(deltaTime);

			// Check if we need to scroll the background
			const uint32& keysHeld = InputManager::GetKeysHeld();
			const uint32& keysPressed = InputManager::GetKeysPressed();
			WorldMap& worldMap = WorldMap::Instance();

			ScrollDir::Type scrollDir = ScrollDir::None;

			if ( keysHeld & KEY_L )
			{
				if ( keysPressed & KEY_LEFT	&& worldMap.GetCurrScreen().x-1 >= 0 )							scrollDir = ScrollDir::Left;
				if ( keysPressed & KEY_RIGHT && worldMap.GetCurrScreen().x+1 < worldMap.GetNumScreensX())	scrollDir = ScrollDir::Right;
				if ( keysPressed & KEY_UP && worldMap.GetCurrScreen().y-1 >= 0 )							scrollDir = ScrollDir::Up;
				if ( keysPressed & KEY_DOWN	&& worldMap.GetCurrScreen().y+1 < worldMap.GetNumScreensY())	scrollDir = ScrollDir::Down;
			}

			Data().mScrollDir = scrollDir;
		}
	};

	struct Scrolling : GameStateBase
	{
		uint16 mScrollX;
		uint16 mScrollY;

		Scrolling()
			: mScrollX(0)
			, mScrollY(0)
		{		
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (IsInState<Scrolling_Done>())
			{
				return SiblingTransition<NormalPlay>();
			}

			return InnerEntryTransition<Scrolling_PreScroll>();
		}

		void UpdateScrollState()
		{
			GraphicsEngine::GetBgLayer(2).SetScroll(mScrollX, mScrollY);
			GraphicsEngine::GetBgLayer(3).SetScroll(mScrollX, mScrollY);
		}
	};

	struct Scrolling_PreScroll : GameStateBase
	{
		virtual void OnEnter()
		{
			Scrolling* pScrollingState = GetState<Scrolling>();
			ASSERT(pScrollingState->mScrollX == 0 && pScrollingState->mScrollY == 0);
			WorldMap& worldMap = WorldMap::Instance();
			
			switch (Data().mScrollDir)
			{
			case ScrollDir::Right:
				worldMap.DrawScreenTiles(worldMap.GetCurrScreen() + Vector2I(1,0), Vector2I(1,0));
				break;

			case ScrollDir::Left:
				worldMap.DrawScreenTiles(worldMap.GetCurrScreen(), Vector2I(1,0));
				pScrollingState->mScrollX = HwScreenSizeX;
				pScrollingState->UpdateScrollState();

				//@HACK: To avoid tearing, wait until our scroll position has been updated before
				// drawing over current screen
				GraphicsEngine::WaitForVBlank();
				GraphicsEngine::PostVBlankUpdate(); // Push bg values

				worldMap.DrawScreenTiles(worldMap.GetCurrScreen() + Vector2I(-1,0), Vector2I(0,0));
				break;

			case ScrollDir::Up:
				worldMap.DrawScreenTiles(worldMap.GetCurrScreen(), Vector2I(0,1));
				pScrollingState->mScrollY = HwScreenSizeY;
				pScrollingState->UpdateScrollState();
				
				//@HACK: Same as above
				GraphicsEngine::WaitForVBlank();
				GraphicsEngine::PostVBlankUpdate(); // Push bg values

				worldMap.DrawScreenTiles(worldMap.GetCurrScreen() + Vector2I(0,-1), Vector2I(0,0));
				break;

			case ScrollDir::Down:
				worldMap.DrawScreenTiles(worldMap.GetCurrScreen() + Vector2I(0,1), Vector2I(0,1));
				break;

			case ScrollDir::None:
			default:
				FAIL();
			}
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<Scrolling_Scroll>();
		}
	};

	struct Scrolling_Scroll : GameStateBase
	{
		bool mIsDoneScrolling;

		virtual void OnEnter()
		{
			mIsDoneScrolling = false;
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (mIsDoneScrolling)
			{
				return SiblingTransition<Scrolling_PostScroll>();
			}
			return NoTransition();
		}
		

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			Scrolling* pScrollingState = GetState<Scrolling>();
			const uint16 scrollDelta = 8 * deltaTime;

			switch (Data().mScrollDir)
			{
			case ScrollDir::Right:
				pScrollingState->mScrollX += scrollDelta;
				pScrollingState->mScrollX = MathEx::Clamp(pScrollingState->mScrollX, 0, HwScreenSizeX);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollX >= HwScreenSizeX);
				break;

			case ScrollDir::Left:
				pScrollingState->mScrollX -= scrollDelta;
				pScrollingState->mScrollX = MathEx::Clamp(pScrollingState->mScrollX, 0, HwScreenSizeX);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollX <= 0);
				break;

			case ScrollDir::Up:
				pScrollingState->mScrollY -= scrollDelta;
				pScrollingState->mScrollY = MathEx::Clamp(pScrollingState->mScrollY, 0, HwScreenSizeY);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollY <= 0);
				break;

			case ScrollDir::Down:
				pScrollingState->mScrollY += scrollDelta;
				pScrollingState->mScrollY = MathEx::Clamp(pScrollingState->mScrollY, 0, HwScreenSizeY);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollY >= HwScreenSizeY);
				break;

			case ScrollDir::None:
			default:
				FAIL();
			}
		}
	};

	struct Scrolling_PostScroll : GameStateBase
	{
		virtual void OnEnter()
		{
			Scrolling* pScrollingState = GetState<Scrolling>();
			WorldMap& worldMap = WorldMap::Instance();
			const Vector2I& currScreen = worldMap.GetCurrScreen();

			switch (Data().mScrollDir)
			{
			case ScrollDir::Right:
				
				worldMap.SetCurrScreen(currScreen + Vector2I(1, 0));
				break;

			case ScrollDir::Left:
				worldMap.SetCurrScreen(currScreen + Vector2I(-1, 0));
				break;

			case ScrollDir::Up:
				worldMap.SetCurrScreen(currScreen + Vector2I(0, -1));
				break;

			case ScrollDir::Down:
				worldMap.SetCurrScreen(currScreen + Vector2I(0, 1));
				break;

			case ScrollDir::None:
			default:
				FAIL();
			}

			// Copy new current screen to 0,0 and reset scroll vars
			WorldMap::Instance().DrawScreenTiles(WorldMap::Instance().GetCurrScreen(), Vector2I(0,0));
			pScrollingState->mScrollX = 0;
			pScrollingState->mScrollY = 0;
			pScrollingState->UpdateScrollState();

			Data().mScrollDir = ScrollDir::None;
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<Scrolling_Done>();
		}
	};

	struct Scrolling_Done : GameStateBase
	{
	};
};

//---------------------------------------------------------------------------------
int main(void) 
{
	ExceptionHandler::EnableDefaultHandler();

	GraphicsEngine::Init(GameMetaTileSizeX, GameMetaTileSizeY);

	LoadAllGameAnimAssets();

	StateMachine gameStateMachine;
	gameStateMachine.SetDebugLevel(1);
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
