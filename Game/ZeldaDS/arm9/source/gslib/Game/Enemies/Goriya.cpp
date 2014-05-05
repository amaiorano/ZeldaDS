#include "Goriya.h"
#include "gslib/Game/EnemyState.h"
#include "gslib/Game/SceneGraph.h"
#include "gslib/Game/Boomerang.h"

typedef StateWithOwner<Goriya, EnemyState> GoriyaState;

struct GoriyaStates
{
	struct Main : GoriyaState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement<GoriyaState>
	{
		typedef EnemySharedStates::RandomMovement<GoriyaState> Base;

		GameTimeType mElapsedTime;
		GameTimeType mTimeToAttack;

		Move()
		{
			Base::SetMinMaxTilesToMove(3, 6);
			Base::SetMoveSpeed(1, 2);
		}

		virtual void OnEnter()
		{
			Base::OnEnter();
			mElapsedTime = 0;
			mTimeToAttack = MathEx::Rand(SEC_TO_FRAMES(2), SEC_TO_FRAMES(4));
		}

		virtual Transition GetTransition()
		{
			if (mElapsedTime > mTimeToAttack)
			{
				return SiblingTransition<Attack>();
			}
			return NoTransition();
		}

		virtual void Update(GameTimeType deltaTime)
		{
			Base::Update(deltaTime);
			mElapsedTime += deltaTime;
		}
	};

	struct Attack : GoriyaState
	{
		virtual void OnEnter()
		{
			ASSERT(!Owner().mpBoomerang);
			const Vector2I& launchDir = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());
			Owner().mpBoomerang = new Boomerang(false);
			Owner().mpBoomerang->Init(&Owner(), launchDir);
			SceneGraph::Instance().AddNode(Owner().mpBoomerang); // Removed from scene in Update() or when leaving root state

			PlayAnim(BaseAnim::Attack);
		}

		virtual Transition GetTransition()
		{
			if (Owner().mpBoomerang->HasReturned())
			{
				return SiblingTransition<Move>();
			}
			return NoTransition();
		}
	};

}; // struct GoriyaStates

Goriya::Goriya()
	: mpBoomerang(NULL)
{
}

Transition Goriya::GetRootTransition()
{
	return InnerEntryTransition<GoriyaStates::Main>();
}

void Goriya::OnDead()
{
	if (mpBoomerang)
	{
		SceneGraph::Instance().RemoveNodePostUpdate(mpBoomerang);
		mpBoomerang = 0;
	}

	Base::OnDead();
}

void Goriya::Update(GameTimeType deltaTime)
{
	Base::Update(deltaTime);

	if (mpBoomerang && mpBoomerang->HasReturned())
	{
		SceneGraph::Instance().RemoveNodePostUpdate(mpBoomerang);
		mpBoomerang = NULL;
	}
}
