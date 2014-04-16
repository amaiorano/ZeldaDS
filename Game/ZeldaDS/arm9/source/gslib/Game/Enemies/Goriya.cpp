#include "Goriya.h"
#include "gslib/Game/EnemyState.h"
#include "gslib/Game/SceneGraph.h"
#include "gslib/Game/Boomerang.h"

struct GoriyaSharedStateData : EnemySharedStateData
{
	GoriyaSharedStateData()
		: mpBoomerang(NULL)
	{
	}

	Boomerang* mpBoomerang;
};

typedef StateWithOwnerAndData<Goriya, GoriyaSharedStateData, EnemyState> GoriyaState;

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
			ASSERT(!Data().mpBoomerang);
			const Vector2I& launchDir = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());
			Data().mpBoomerang = new Boomerang(false);
			Data().mpBoomerang->Init(&Owner(), launchDir);
			SceneGraph::Instance().AddNode(Data().mpBoomerang); // Removed from scene in Update() or when leaving root state

			PlayAnim(BaseAnim::Attack);
		}

		virtual Transition GetTransition()
		{
			if (Data().mpBoomerang->HasReturned())
			{
				return SiblingTransition<Move>();
			}
			return NoTransition();
		}
	};

}; // struct GoriyaStates

SharedStateData* Goriya::CreateSharedStateData()
{
	return new GoriyaSharedStateData();
}

Transition Goriya::GetRootTransition()
{
	return InnerEntryTransition<GoriyaStates::Main>();
}

void Goriya::OnDead()
{
	Boomerang*& pBoomerang = static_cast<GoriyaSharedStateData*>(mpSharedStateData)->mpBoomerang;
	if (pBoomerang)
	{
		SceneGraph::Instance().RemoveNodePostUpdate(pBoomerang);
		pBoomerang = 0;
	}

	Base::OnDead();
}

void Goriya::Update(GameTimeType deltaTime)
{
	Base::Update(deltaTime);

	Boomerang*& pBoomerang = static_cast<GoriyaSharedStateData*>(mpSharedStateData)->mpBoomerang;
	if (pBoomerang && pBoomerang->HasReturned())
	{
		SceneGraph::Instance().RemoveNodePostUpdate(pBoomerang);
		pBoomerang = NULL;
	}
}
