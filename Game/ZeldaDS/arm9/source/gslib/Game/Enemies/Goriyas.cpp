#include "Goriyas.h"
#include "gslib/Game/EnemyState.h"

struct GoriyasStates
{
	struct Main : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement
	{
		HsmTimeType mElapsedTime;
		HsmTimeType mTimeToAttack;

		virtual void OnEnter()
		{
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

	struct Attack : EnemyStateBase
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Attack);
			//@TODO: Throw boomerang
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			//@TODO: When boomerang returns, go back to Move
			if (IsAnimFinished())
			{
				return SiblingTransition<Move>();
			}
			return NoTransition();
		}
	};

}; // struct GoriyasStates

Transition& Goriyas::GetRootTransition()
{
	return InnerEntryTransition<GoriyasStates::Main>();
}
