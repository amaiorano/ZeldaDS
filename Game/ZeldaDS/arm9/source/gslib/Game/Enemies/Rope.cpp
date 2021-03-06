#include "Rope.h"
#include "gslib/Game/EnemyState.h"
#include "gslib/Game/Player.h"
#include "gslib/Game/SceneGraph.h"
#include "gslib/Hw/Constants.h"

typedef StateWithOwner<Rope, EnemyState> RopeState;

struct RopeStates
{
	// Rope moves around randomly, but as soon as it sees the player, it strikes in the players
	// direction until it collides, at which point it resumes random movement

	struct Main : RopeState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement<>
	{
		typedef EnemySharedStates::RandomMovement<> Base;
		bool mShouldStrike;

		Move() : mShouldStrike(false)
		{
		}

		virtual Transition GetTransition()
		{
			if (mShouldStrike)
			{
				return SiblingTransition<Attack>();
			}
			return NoTransition();
		}

		virtual void Update(GameTimeType deltaTime)
		{
			Base::Update(deltaTime); // Move

			// Due to 8x8 grid movement, we may not be facing our desired direction, so we only
			// strike when we're on the grid line
			const bool facingDesiredDirection = mDesiredDir == Owner().GetSpriteDir();
			if (facingDesiredDirection)
			{
				// Project a bounding box in front of myself and see if it collides with the player
				BoundingBox fwdBBox = Owner().GetBoundingBox();
				
				switch (mDesiredDir)
				{
				case SpriteDir::Left:
					fwdBBox.pos.x -= HwScreenSizeX;
					fwdBBox.w = HwScreenSizeX;
					break;

				case SpriteDir::Right:
					fwdBBox.pos.x += Owner().GetWidth();
					fwdBBox.w = HwScreenSizeX;
					break;

				case SpriteDir::Up:
					fwdBBox.pos.y -= HwScreenSizeY;
					fwdBBox.h = HwScreenSizeY;
					break;

				case SpriteDir::Down:
					fwdBBox.pos.y += Owner().GetHeight();
					fwdBBox.h = HwScreenSizeY;
					break;

				default: FAIL(); break;
				}

				const Player* pPlayer = SceneGraph::Instance().GetPlayerList().front(); //@TODO: iterate for all players
				mShouldStrike = fwdBBox.CollidesWith(pPlayer->GetBoundingBox());

				//extern void DrawQuad(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color, uint16 alpha);
				//fwdBBox.pos = Camera::Instance().WorldToScreen(fwdBBox.pos);
				//DrawQuad(fwdBBox.pos.x, fwdBBox.pos.y, fwdBBox.w, fwdBBox.h, RGB15(255,0,0), 15);
			}
		}
	};

	struct Attack : RopeState
	{
		virtual Transition GetTransition()
		{
			// If we hit anything (player or world), stop attacking
			if (Owner().mLastFrameCollision.mIsSet 
				|| !GameHelpers::IsPhysicalInScreenBounds(Owner())
				)
			{
				// Change direction to make sure we don't keep striking the player
				Owner().SetSpriteDir( GameHelpers::GetRandomSpriteDir(Owner().GetSpriteDir()) );

				return SiblingTransition<Move>();
			}

			return InnerEntryTransition<Alert>();
		}
	};

	struct Alert : RopeState
	{
		GameTimeType mElapsedTime;

		virtual void OnEnter()
		{
			mElapsedTime = 0;
			PlayAnim(BaseAnim::Idle);
		}

		virtual Transition GetTransition()
		{			
			if (mElapsedTime > SEC_TO_FRAMES(0.3f))
			{
				return SiblingTransition<Strike>();
			}

			return NoTransition();
		}

		virtual void Update(GameTimeType deltaTime)
		{
			mElapsedTime += deltaTime;
		}
	};

	struct Strike : RopeState
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Attack);
		}

		virtual void Update(GameTimeType deltaTime)
		{
			const int16 moveSpeed = 2; // pixels/frame
			const Vector2I& dirVec = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());
			Owner().SetVelocity(dirVec * moveSpeed);
		}
	};

}; // struct RopeStates

Transition Rope::GetRootTransition()
{
	return InnerEntryTransition<RopeStates::Main>();
}
