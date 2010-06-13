#include "GameAnims.h"
#include "gslib/Core/Core.h"

#include "data/characters.h"
#include "data/items.h"

namespace
{
	// Helper to easily build an AnimAsset in code
	class AnimAssetBuilder
	{
	public:
		typedef AnimAssetBuilder ThisType;

		ThisType& CreateAndAdd(AnimAssetKey key)
		{
			mpAnimAsset = new AnimAsset();
			AnimAssetManager::AddAnimAsset(key, mpAnimAsset);
			return *this;
		}

		ThisType& InitPoses(uint8* pFrameGfx, uint16 frameSize)
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


	// Helper to create and add regular cyclic animations for all directions (the majority of cases)
	void CreateAndAddAnimAssets(
		GameActor::Type gameActor,
		BaseAnim::Type baseAnim, 
		uint8* pFrameGfx,
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

			//AnimAsset* pAnimAsset = new AnimAsset();
			//pAnimAsset->mAnimPoses.Init(pFrameGfx, frameSize);
			//pAnimAsset->mAnimTimeline.Populate(firstFrameIndex, numFrames, unitsPerFrame, animCycle);

			//AnimAssetManager::AddAnimAsset(MakeAnimAssetKey(gameActor, baseAnim, (SpriteDir::Type)d), pAnimAsset);
		}
	}
}



void LoadAllGameAnimAssets()
{
	//@TODO: Externalize into data files
	//@TODO: Replace SEC_TO_FRAMES() with actual frames below to reduce noise

	using namespace GameActor;
	using namespace BaseAnim;
	using namespace SpriteDir;
	
	AnimAssetBuilder builder;

	const int NumTilesPerRow = 14;
	
	uint8* pFrameGfx = (uint8*)charactersTiles;
	int FrameSize = 16 * 16; // 8bpp sprites

	// General spawn anim
	builder.CreateAndAdd(MakeAnimAssetKey(GameActor::None, Spawn, SpriteDir::None))
		.InitPoses(pFrameGfx, FrameSize)
		.BuildTimeline()(178, SEC_TO_FRAMES(0.1f))(179, SEC_TO_FRAMES(0.1f))(180, SEC_TO_FRAMES(0.1f))(181, SEC_TO_FRAMES(0.1f));

	// Manually add the Idle anims (which include a blink frame)
	{
		builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Right)).InitPoses	(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(1, SEC_TO_FRAMES(2.0f))(22, SEC_TO_FRAMES(0.2f));
		builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Left )).InitPoses(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(4, SEC_TO_FRAMES(2.0f))(23, SEC_TO_FRAMES(0.2f));
		builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Down )).InitPoses(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(7, SEC_TO_FRAMES(2.0f))(24, SEC_TO_FRAMES(0.2f));
		builder.CreateAndAdd(MakeAnimAssetKey(Hero, Idle, Up   )).InitPoses(pFrameGfx, FrameSize).BuildTimeline().Looping(true)(10, SEC_TO_FRAMES(0.0f));
	}

	CreateAndAddAnimAssets(Hero, Move, pFrameGfx, FrameSize, 0 * NumTilesPerRow, 3, 0, SEC_TO_FRAMES(0.15f), AnimCycle::PingPong);
	CreateAndAddAnimAssets(Hero, Attack, pFrameGfx, FrameSize, 1 * NumTilesPerRow, 2, 0, SEC_TO_FRAMES(0.15f), AnimCycle::Once);
	CreateAndAddAnimAssets(Hero, UseItem, pFrameGfx, FrameSize, 1 * NumTilesPerRow + 1, 1, 1, SEC_TO_FRAMES(0.20f), AnimCycle::Once);

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

	// These enemies only move
	CreateAndAddAnimAssets(Snake,    Idle, pFrameGfx, FrameSize, (2 * NumTilesPerRow), 1, 2, SEC_TO_FRAMES(0.0f), AnimCycle::Once);
	CreateAndAddAnimAssets(Snake,    Move, pFrameGfx, FrameSize, 2 * NumTilesPerRow, 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
	CreateAndAddAnimAssets(Snake,    Attack, pFrameGfx, FrameSize, (2 * NumTilesPerRow)+2, 1, 2, SEC_TO_FRAMES(0.25f), AnimCycle::Once);
	
	CreateAndAddAnimAssets(Knight,   Move, pFrameGfx, FrameSize, 3 * NumTilesPerRow, 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
	CreateAndAddAnimAssets(Skeleton, Move, pFrameGfx, FrameSize, 4 * NumTilesPerRow, 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
	
	CreateAndAddAnimAssets(Goriyas,  Idle, pFrameGfx, FrameSize, (5 * NumTilesPerRow)+1, 1, 2, SEC_TO_FRAMES(0.0f), AnimCycle::Loop);
	CreateAndAddAnimAssets(Goriyas,  Move, pFrameGfx, FrameSize, 5 * NumTilesPerRow, 3, 0, SEC_TO_FRAMES(0.25f), AnimCycle::PingPong);
	CreateAndAddAnimAssets(Goriyas,  Attack, pFrameGfx, FrameSize, (5 * NumTilesPerRow)+12, 1, 0, SEC_TO_FRAMES(0.5f), AnimCycle::Once);


	// Item animations
	pFrameGfx = (uint8*)itemsTiles;

	// Boomerang
	{
		const int numFramesPerImage = 5;
		builder.CreateAndAdd(MakeAnimAssetKey(Boomerang, ItemDefault, SpriteDir::None))
			.InitPoses(pFrameGfx, FrameSize)
			.BuildTimeline().Looping(true)(4, numFramesPerImage)(5, numFramesPerImage)(6, numFramesPerImage)(7, numFramesPerImage);
	}
}
