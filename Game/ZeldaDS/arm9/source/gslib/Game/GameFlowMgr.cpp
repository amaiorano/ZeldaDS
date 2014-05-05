#include "GameFlowMgr.h"

#include "gslib/Math/MathEx.h"

#include "gslib/Hw/FileSystem.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/SpriteRenderGroupMgr.h"
#include "gslib/Hw/AudioEngine.h"

#include "gslib/Game/SceneGraph.h"
#include "gslib/Game/PhysicsSimulator.h"
#include "gslib/Game/ScrollingMgr.h"
#include "gslib/Game/WorldMap.h"
#include "gslib/Game/GameAnims.h"
#include "gslib/Game/Player.h"
#include "gslib/Game/Enemy.h"
#include "gslib/Game/Weapon.h"
#include "gslib/Game/EnemyFactory.h"
#include "gslib/Game/GameHelpers.h"
#include "gslib/Game/GameResources.h"

#include "data/soundbank.h"

#include <string>

using namespace hsm;

typedef StateWithOwner<GameFlowMgr> GameFlowState;

struct GameFlowStates
{
	struct Root : GameFlowState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<InitGame>();
		}
	};

	struct InitGame : GameFlowState
	{
		virtual void OnEnter()
		{
			// Allocate resource buffers - we keep these allocated for the lifetime of the game
			for (int i = 0; i < GameResource::NumTypes; ++i)
			{
				ResourceMgr::Instance().AllocateResourceBuffer(i, GameResource::SizeBytesPerType[i]);
			}

			// Load resources - not sure if some of these would change based on current map (characters?)
			ResourceMgr::Instance().LoadResource(GameResource::Gfx_Items, "Graphics/items.img");
			ResourceMgr::Instance().LoadResource(GameResource::Gfx_Characters16x16, "Graphics/characters.img");
			ResourceMgr::Instance().LoadResource(GameResource::Gfx_Characters32x32, "Graphics/characters_32x32.img");

			// Load sprite palette
			ResourceData data = ResourceMgr::Instance().LoadResource(GameResource::Temporary, "Graphics/sprite_palette.pal");
			GraphicsEngine::LoadSpritePalette(data.DataAs<const uint16*>(), data.SizeBytes());

			GraphicsEngine::SetBgFontColor( RGB8(255, 255, 255) );
			GraphicsEngine::SetSubBgFontColor( RGB8(255, 255, 255) );

			static SpriteRenderGroup groups[] =
			{
				{ GameSpriteRenderGroup::AboveAll, 2 },
				{ GameSpriteRenderGroup::Heroes, 1 },
				{ GameSpriteRenderGroup::Weapons, 20 },
				{ GameSpriteRenderGroup::Enemies, 20 },
			};
			CT_ASSERT(NUM_ARRAY_ELEMS(groups) == GameSpriteRenderGroup::NumTypes);
			
			SpriteRenderGroupMgr::Instance().Init(groups, GameSpriteRenderGroup::NumTypes);

			LoadAllGameAnimAssets();

	
			//GraphicsEngine::GetBgLayer(0).ActivateTextLayer();
			//printf("Testing text on bg layer 0!\n");

			AudioEngine::LoadBank("Audio/soundbank.bin");

			ScrollingMgr::Instance().Init();
		}
		
		virtual Transition GetTransition()
		{
			return SiblingTransition<EnableRendering>();
		}
	};

	struct EnableRendering : GameFlowState
	{
		uint16 mNumElapsedFrames;

		EnableRendering() : mNumElapsedFrames(0)
		{
		}

		virtual void OnEnter()
		{
			// Before enabling backgrounds, start faded out
			GraphicsEngine::FadeScreen(FadeScreenDir::Out, 0);
		}

		virtual Transition GetTransition()
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

	struct LoadMap : GameFlowState
	{
		virtual void OnEnter()
		{
			WorldMap& worldMap = WorldMap::Instance();
			ASSERT(!Owner().mTargetWorldMap.empty());
			worldMap.LoadMap(Owner().mTargetWorldMap.c_str());
			Owner().mCurrWorldMap = Owner().mTargetWorldMap;
			Owner().mTargetWorldMap.clear();

			SceneGraph::Instance().SetWorldMap(worldMap);

			ResourceData data;

			data = ResourceMgr::Instance().LoadResource(GameResource::Temporary, "Graphics/tile_palette.pal");
			GraphicsEngine::LoadBgPalette(data.DataAs<const uint16*>(), data.SizeBytes());

			// Mapping corresponds to GameTileSetGroups.xml (Zelous)
			switch (worldMap.GetTileSetGroupIndex())
			{
			case 0: // Overworld
				data = ResourceMgr::Instance().LoadResource(GameResource::Temporary, "Graphics/overworld_fg.img");
				GraphicsEngine::GetBgLayer(2).LoadTilesImage(data.DataAs<const uint16*>(), data.SizeBytes());

				data = ResourceMgr::Instance().LoadResource(GameResource::Temporary, "Graphics/overworld_bg.img");
				GraphicsEngine::GetBgLayer(3).LoadTilesImage(data.DataAs<const uint16*>(), data.SizeBytes());

				break;

			case 1: // Dungeon
				data = ResourceMgr::Instance().LoadResource(GameResource::Temporary, "Graphics/dungeon_fg.img");
				GraphicsEngine::GetBgLayer(2).LoadTilesImage(data.DataAs<const uint16*>(), data.SizeBytes());

				data = ResourceMgr::Instance().LoadResource(GameResource::Temporary, "Graphics/dungeon_bg.img");
				GraphicsEngine::GetBgLayer(3).LoadTilesImage(data.DataAs<const uint16*>(), data.SizeBytes());

				break;
			}

			AudioEngine::SetMusicVolume(1.0f);
			AudioEngine::PlayMusic(MOD_OVERWORLD3);

			// Use map's spawn position or a specified pos?
			Vector2I targetWorldPos = Owner().mTargetWorldPos;
			if (Owner().mTargetWorldPos == UseMapSpawnPosition)
			{
				const WorldMap::PlayerSpawnData& playerSpawnData = worldMap.GetPlayerSpawnData();
				targetWorldPos = playerSpawnData.mPos;
			}

			//@TODO: Need to load player save data
			Player* pPlayer = new Player();
			
			pPlayer->Init(targetWorldPos);
			SceneGraph::Instance().AddNode(pPlayer);

			const Vector2I& startScreen = WorldMap::WorldPosToScreen(targetWorldPos);
			ScrollingMgr::Instance().Reset(startScreen);

			// After a few map reloads, we should be getting a 0 byte delta...
			//HEAP_REPORT_DELTA_SIZE();
			//HEAP_CHECK_SIZE();
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<PlayingMap>();
		}
	};

	struct PlayingMap : GameFlowState, IScrollingEventListener
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
			
			RemoveAllNodesImmediately();
		}

		virtual Transition GetTransition()
		{
			//@TODO TEMP HACK: L+R toggle map loading between 2 test maps
			if ((InputManager::GetKeysHeld() & (KEY_L|KEY_R)) == (KEY_L|KEY_R))
			{
				if (Owner().mCurrWorldMap == "Maps/TestMap.map")
				{
					Owner().mTargetWorldMap = "Maps/TestMap2.map";
				}
				else
				{
					Owner().mTargetWorldMap = "Maps/TestMap.map";
				}
			}

			if ( !Owner().mTargetWorldMap.empty() )
			{
				return SiblingTransition<LeavingMap>();
			}
			return NoTransition();
		}

		virtual void Update(GameTimeType deltaTime)
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
			RemoveAllNodesExceptPlayer();
		}

		virtual void OnScrollingEnd()
		{
			mSpawnEnemiesNextFrame = true;
		}

		void RemoveAllNodesExceptPlayer()
		{
			Player* pPlayer = SceneGraph::Instance().GetPlayerList().front();
			SceneNodeList& sceneNodes = SceneGraph::Instance().GetSceneNodeList();
			for (SceneNodeList::iterator iter = sceneNodes.begin(); iter != sceneNodes.end(); ++iter)
			{
				if (*iter == pPlayer)
				{
					continue;
				}

				SceneGraph::Instance().RemoveNodePostUpdate(*iter);
			}
		}

		void RemoveAllNodesImmediately()
		{
			RemoveAllNodesExceptPlayer();

			Player* pPlayer = SceneGraph::Instance().GetPlayerList().front();
			SceneGraph::Instance().RemoveNodePostUpdate(pPlayer);

			// For removal immediately
			SceneGraph::Instance().Update(0);
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

	struct LeavingMap : GameFlowState
	{
		virtual void OnEnter()
		{
			GraphicsEngine::FadeScreen(FadeScreenDir::Out, SEC_TO_FRAMES(0.5f));
		}

		virtual void OnExit()
		{
			AudioEngine::SetMusicVolume(1.0f);
		}

		virtual Transition GetTransition()
		{
			if ( !GraphicsEngine::IsFadingScreen() )
			{
				return SiblingTransition<UnloadMap>();
			}
			return NoTransition();
		}

		virtual void Update(GameTimeType deltaTime)
		{
			AudioEngine::SetMusicVolume( GraphicsEngine::GetFadeRatio() );
		}
	};

	struct UnloadMap : GameFlowState
	{
		virtual void OnEnter()
		{
			AudioEngine::StopMusic();
			WorldMap::Instance().UnloadMap();
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<LoadMap>();
		}
	};

}; // struct GameFlowStates


GameFlowMgr::GameFlowMgr()
	: mTargetWorldPos(InitZero)
{
}

void GameFlowMgr::Init()
{
	mStateMachine.Initialize<GameFlowStates::Root>(this, "GameFlowMgr");
}

void GameFlowMgr::Update(GameTimeType deltaTime)
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates(deltaTime);
}

void GameFlowMgr::SetTargetWorldMap(const char* worldMapFilePath, const Vector2I& initialPos)
{
	mTargetWorldMap = worldMapFilePath;
	mTargetWorldPos = initialPos;
}
