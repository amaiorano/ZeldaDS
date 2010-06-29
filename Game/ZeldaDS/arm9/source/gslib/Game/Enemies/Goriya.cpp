#include "Goriyas.h"
#include "gslib/Game/EnemyState.h"
#include "gslib/Game/SceneGraph.h"
#include "gslib/Game/Boomerang.h"

struct GoriyaStates
{
	struct GoriyaSharedStateData : EnemySharedStateData
	{
		Boomerang mBoomerang;
	};

	typedef CharacterStateBase<GoriyaSharedStateData, Goriya> GoriyaStateBase;

	struct Main : GoriyaStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement<GoriyaStateBase>
	{
		typedef EnemySharedStates::RandomMovement<GoriyaStateBase> Base;

		HsmTimeType mElapsedTime;
		HsmTimeType mTimeToAttack;

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

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			mElapsedTime += deltaTime;
			if (mElapsedTime > mTimeToAttack)
			{
				return SiblingTransition<Attack>();
			}
			return NoTransition();
		}
	};

	struct Attack : GoriyaStateBase
	{
		virtual void OnEnter()
		{
			ASSERT(!Data().mBoomerang.IsNodeInScene());
			const Vector2I& launchDir = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());
			Data().mBoomerang.Init(&Owner(), launchDir);
			SceneGraph::Instance().AddNode(Data().mBoomerang); // Removed from scene in Update() or when leaving root state

			PlayAnim(BaseAnim::Attack);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (Data().mBoomerang.HasReturned())
			{
				return SiblingTransition<Move>();
			}
			return NoTransition();
		}
	};

}; // struct GoriyaStates

EnemySharedStateData* Goriya::CreateSharedStateData()
{
	return new GoriyaStates::GoriyaSharedStateData();
}

Transition& Goriya::GetRootTransition()
{
	return InnerEntryTransition<GoriyaStates::Main>();
}

void Goriya::OnDead()
{
	//@TODO: Need a way to transfer ownership of ISceneNodes to SceneGraph entirely so that
	// if, for example, the enemy is destroyed before the boomerang, the boomerang pointer
	// in SceneGraph won't be dangling. The only reason this works now is because we schedule
	// the boomerang's removal before that of the enemy.
	Boomerang& boomerang = static_cast<GoriyaStates::GoriyaSharedStateData*>(mpSharedStateData)->mBoomerang;
	if (boomerang.IsNodeInScene())
	{
		SceneGraph::Instance().RemoveNodePostUpdate(boomerang);
	}

	Base::OnDead();
}

void Goriya::Update(GameTimeType deltaTime)
{
	Base::Update(deltaTime);

	Boomerang& boomerang = static_cast<GoriyaStates::GoriyaSharedStateData*>(mpSharedStateData)->mBoomerang;
	if (boomerang.IsNodeInScene() && boomerang.HasReturned())
	{
		SceneGraph::Instance().RemoveNodePostUpdate(boomerang);
	}
}
