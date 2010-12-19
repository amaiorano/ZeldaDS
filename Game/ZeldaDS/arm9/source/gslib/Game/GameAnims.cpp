#include "GameAnims.h"
#include "gslib/Core/Core.h"

#include "gslib/Game/GameResources.h"

namespace
{
	// Helper to easily build an AnimAsset in code
	class AnimAssetBuilder
	{
	public:
		typedef AnimAssetBuilder ThisType;

		ThisType& CreateAndAdd(AnimAssetKey key)
		{
			//@TODO: This allocation is never deleted! We're ok for now since we only
			// allocate anim assets once at Init time, but we should clean this up somewhere.
			mpAnimAsset = new AnimAsset();
			AnimAssetManager::AddAnimAsset(key, mpAnimAsset);
			return *this;
		}

		ThisType& InitPoses(const uint8* pFrameGfx, uint16 frameSize)
		{
			mpAnimAsset->mAnimPoses.Init(pFrameGfx, frameSize);
			return *this;
		}

		ThisType& InitTimeline(int firstFrameIndex, int numFrames, AnimTimeType unitsPerFrame, AnimCycle::Type animCycle)
		{
			mpAnimAsset->mAnimTimeline.Populate(firstFrameIndex, numFrames, unitsPerFrame, animCycle);
			return *this;
		}	

		struct TimelineBuilder
		{
			typedef TimelineBuilder ThisType;

			TimelineBuilder(AnimTimeline& timeline) : mpTimeline(&timeline)
			{
				ASSERT(mpTimeline);
			}

			ThisType& Looping(bool looping)
			{
				mpTimeline->SetLooping(looping);
				return *this;
			}

			// Add frames with operator()
			ThisType& operator()(int frameIndex, AnimTimeType frameDuration)
			{
				mpTimeline->AddKeyFrame(frameIndex, frameDuration);
				return *this;
			}

		private:
			AnimTimeline* mpTimeline;
		};

		TimelineBuilder BuildTimeline()
		{
			return TimelineBuilder(mpAnimAsset->mAnimTimeline);
		}

	private:
		AnimAsset* mpAnimAsset;
	};

	// Helper to create and add four-directional animations
	void CreateAndAddAnimAssets4(
		GameActor::Type gameActor,
		BaseAnim::Type baseAnim, 
		const uint8* pFrameGfx,
		uint16 frameSize,
		int firstRightFrameIndex,
		int numFrames,
		int framesToSkip,
		AnimTimeType unitsPerFrame,
		AnimCycle::Type animCycle)
	{
		AnimAssetBuilder builder;

		for (int d=0; d < SpriteDir::NumTypes; ++d)
		{
			const int firstFrameIndex = firstRightFrameIndex + d * (numFrames + framesToSkip);

			builder.CreateAndAdd(MakeAnimAssetKey(gameActor, baseAnim, (SpriteDir::Type)d))
				.InitPoses(pFrameGfx, frameSize)
				.InitTimeline(firstFrameIndex, numFrames, unitsPerFrame, animCycle);
		}
	}

	// Helper to create and add non-directional animations
	void CreateAndAddAnimAssets1(
		GameActor::Type gameActor,
		BaseAnim::Type baseAnim, 
		const uint8* pFrameGfx,
		uint16 frameSize,
		int firstFrameIndex,
		int numFrames,
		AnimTimeType unitsPerFrame,
		AnimCycle::Type animCycle)
	{
		AnimAssetBuilder builder;

		builder.CreateAndAdd(MakeAnimAssetKey(gameActor, baseAnim, SpriteDir::None))
			.InitPoses(pFrameGfx, frameSize)
			.InitTimeline(firstFrameIndex, numFrames, unitsPerFrame, animCycle);

	}

} // anonymous namespace

void LoadAllGameAnimAssets()
{
	//@TODO: Externalize into data files
	//@TODO: Replace SEC_TO_FRAMES() with actual frames below to reduce noise

	using namespace GameActor;
	using namespace BaseAnim;
	using namespace SpriteDir;
	
	AnimAssetBuilder builder;

	// 16x16 characters
	{
		const uint8* pFrameGfx = ResourceMgr::Instance().GetResource(GameResource::Gfx_Characters16x16).Data();
		const int FrameSize = 16 * 16; // 8bpp sprites
		const int NumTilesPerRow = 14;

		// General spawn anim
		CreateAndAddAnimAssets1(GameActor::None, Spawn, pFrameGfx, FrameSize, 178, 4, SEC_TO_FRAMES(0.1f), AnimCycle::Once);

		// Manually add the Idle anims (which include a blink frame)
		{
			builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Right)).InitPoses	(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(1, SEC_TO_FRAMES(2.0f))(22, SEC_TO_FRAMES(0.2f));
			builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Left )).InitPoses(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(4, SEC_TO_FRAMES(2.0f))(23, SEC_TO_FRAMES(0.2f));
			builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Down )).InitPoses(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(7, SEC_TO_FRAMES(2.0f))(24, SEC_TO_FRAMES(0.2f));
			builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Up   )).InitPoses(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(10, SEC_TO_FRAMES(0.0f));
		}

		CreateAndAddAnimAssets4(Hero, Move, pFrameGfx, FrameSize, (0 * NumTilesPerRow + 0), 3, 0, SEC_TO_FRAMES(0.15f), AnimCycle::PingPong);
		CreateAndAddAnimAssets4(Hero, Attack, pFrameGfx, FrameSize, (1 * NumTilesPerRow + 0), 2, 0, SEC_TO_FRAMES(0.10f), AnimCycle::Once);
		CreateAndAddAnimAssets4(Hero, UseItem, pFrameGfx, FrameSize, (1 * NumTilesPerRow + 1), 1, 1, SEC_TO_FRAMES(0.20f), AnimCycle::Once);

		// Player Death anim (must face down)
		{
			// 7: down
			// 4: left
			// 10: up
			// 1: right
			builder.CreateAndAdd(MakeAnimAssetKey(Hero, Die, SpriteDir::None))
				.InitPoses(pFrameGfx, FrameSize)
				.BuildTimeline()
				(7, SEC_TO_FRAMES(0.5f))(4, SEC_TO_FRAMES(0.05f))(10, SEC_TO_FRAMES(0.05f))(1, SEC_TO_FRAMES(0.05f))
				(7, SEC_TO_FRAMES(0.05f))(4, SEC_TO_FRAMES(0.05f))(10, SEC_TO_FRAMES(0.05f))(1, SEC_TO_FRAMES(0.05f))
				(7, SEC_TO_FRAMES(0.05f))(4, SEC_TO_FRAMES(0.05f))(10, SEC_TO_FRAMES(0.05f))(1, SEC_TO_FRAMES(0.05f))
				(7, SEC_TO_FRAMES(0.10f))(4, SEC_TO_FRAMES(0.10f))(10, SEC_TO_FRAMES(0.10f))(1, SEC_TO_FRAMES(0.10f))
				(7, SEC_TO_FRAMES(0.15f))(4, SEC_TO_FRAMES(0.15f))(10, SEC_TO_FRAMES(0.15f))(1, SEC_TO_FRAMES(0.15f))
				(7, SEC_TO_FRAMES(0.15f));
		}

		// Enemies - note that Idle is used for stunned (enemies don't really idle)
		CreateAndAddAnimAssets4(Rope, Idle, pFrameGfx, FrameSize, (2 * NumTilesPerRow + 0), 1, 2, SEC_TO_FRAMES(0.0f), AnimCycle::Once);
		CreateAndAddAnimAssets4(Rope, Move, pFrameGfx, FrameSize, (2 * NumTilesPerRow + 0), 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
		CreateAndAddAnimAssets4(Rope, Attack, pFrameGfx, FrameSize, (2 * NumTilesPerRow + 2), 1, 2, SEC_TO_FRAMES(0.25f), AnimCycle::Once);
		
		CreateAndAddAnimAssets4(Darknut, Idle, pFrameGfx, FrameSize, (3 * NumTilesPerRow + 1), 1, 2, SEC_TO_FRAMES(0.25f), AnimCycle::Loop);
		CreateAndAddAnimAssets4(Darknut, Move, pFrameGfx, FrameSize, (3 * NumTilesPerRow + 0), 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
		
		CreateAndAddAnimAssets4(Stalfos, Idle, pFrameGfx, FrameSize, (4 * NumTilesPerRow + 1), 1, 2, SEC_TO_FRAMES(0.25f), AnimCycle::Loop);
		CreateAndAddAnimAssets4(Stalfos, Move, pFrameGfx, FrameSize, (4 * NumTilesPerRow + 0), 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
		
		CreateAndAddAnimAssets4(Goriya, Idle, pFrameGfx, FrameSize, (5 * NumTilesPerRow + 1), 1, 2, SEC_TO_FRAMES(0.0f), AnimCycle::Loop);
		CreateAndAddAnimAssets4(Goriya, Move, pFrameGfx, FrameSize, (5 * NumTilesPerRow + 0), 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
		CreateAndAddAnimAssets4(Goriya, Attack, pFrameGfx, FrameSize, (5 * NumTilesPerRow + 12), 1, 0, SEC_TO_FRAMES(0.5f), AnimCycle::Once);

		CreateAndAddAnimAssets1(Gel, Idle, pFrameGfx, FrameSize, (6 * NumTilesPerRow + 5), 1, SEC_TO_FRAMES(1.0f), AnimCycle::Loop);
		CreateAndAddAnimAssets1(Gel, Move, pFrameGfx, FrameSize, (6 * NumTilesPerRow + 5), 3, SEC_TO_FRAMES(0.25f), AnimCycle::Loop);

		CreateAndAddAnimAssets1(Keese, Idle, pFrameGfx, FrameSize, (6 * NumTilesPerRow + 8), 1, SEC_TO_FRAMES(1.0f), AnimCycle::Loop);
		CreateAndAddAnimAssets1(Keese, Move, pFrameGfx, FrameSize, (6 * NumTilesPerRow + 8), 3, SEC_TO_FRAMES(0.25f), AnimCycle::Loop);

		CreateAndAddAnimAssets1(WallMaster, Idle, pFrameGfx, FrameSize, (6 * NumTilesPerRow + 2), 1, SEC_TO_FRAMES(1.0f), AnimCycle::Loop);
		CreateAndAddAnimAssets1(WallMaster, Move, pFrameGfx, FrameSize, (6 * NumTilesPerRow + 2), 3, SEC_TO_FRAMES(0.25f), AnimCycle::Loop);
	}

	// 32x32 characters
	{
		const uint8* pFrameGfx = ResourceMgr::Instance().GetResource(GameResource::Gfx_Characters32x32).Data();
		const int FrameSize = 32 * 32; // 8bpp sprites
		const int NumTilesPerRow = 4;

		// Override global Spawn with one that fits 32x32 sprites. What sucks is we need to do this for every 32x32 character.
		//@TODO: Rename BaseAnim::Spawn to Spawn16x16, add BaseAnim::Spawn32x32, create its global anim here, and in the Enemy
		// state machine, call the specific spawn anim based on the sprite dimenions.
		CreateAndAddAnimAssets1(Dragon, Spawn, pFrameGfx, FrameSize, (0 * NumTilesPerRow + 0), 4, SEC_TO_FRAMES(0.1f), AnimCycle::Once);

		CreateAndAddAnimAssets1(Dragon, Idle, pFrameGfx, FrameSize, (1 * NumTilesPerRow + 0), 1, SEC_TO_FRAMES(1.0f), AnimCycle::Loop);
		CreateAndAddAnimAssets1(Dragon, Move, pFrameGfx, FrameSize, (1 * NumTilesPerRow + 0), 3, SEC_TO_FRAMES(0.25f), AnimCycle::Loop);
	}

	// 16x16 items
	{
		const uint8* pFrameGfx = ResourceMgr::Instance().GetResource(GameResource::Gfx_Items).Data();
		const int FrameSize = 16 * 16; // 8bpp sprites

		// Boomerang
		{
			const int numFramesPerImage = 5;
			builder.CreateAndAdd(MakeAnimAssetKey(Boomerang, ItemDefault, SpriteDir::None))
				.InitPoses(pFrameGfx, FrameSize)
				.BuildTimeline().Looping(true)(4, numFramesPerImage)(5, numFramesPerImage)(6, numFramesPerImage)(7, numFramesPerImage);
		}
	}
}
