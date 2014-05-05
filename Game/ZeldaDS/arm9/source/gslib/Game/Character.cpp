#include "Character.h"
#include "CharacterState.h"
#include "GameHelpers.h"
#include "gslib/Core/Rtti.h"

Character::Character()
{
}

void Character::Init(const Vector2I& initPos)
{
	mInitPos = initPos;
	
	InitStateMachine();

	// Child class must initialize state machine
	ASSERT(mStateMachine.IsInitialized());
}

void Character::OnAddToScene()
{
	Base::OnAddToScene();

	SetPosition(mInitPos);
	mDamageInfo.Reset();
	PlayAnim(BaseAnim::Idle); // Set initial pose
	SetSpriteDir(SpriteDir::Down); // Set initial direction
}

void Character::Update(GameTimeType deltaTime)
{
	if (deltaTime > 0) // Total cop out, we don't handle deltaTime == 0 very well
	{
		// Character's must set their velocity every frame to move
		SetVelocity(InitZero);

		mStateMachine.ProcessStateTransitions();
		mStateMachine.UpdateStates(deltaTime);

		mHealth.Update(deltaTime);
	}

	Base::Update(deltaTime); // Updates anim and render state

	OuterToInnerIterator iter = mStateMachine.BeginOuterToInner();
	OuterToInnerIterator end = mStateMachine.EndOuterToInner();
	for ( ; iter != end; ++iter)
	{
		static_cast<CharacterState*>(*iter)->PostAnimUpdate( deltaTime );
	}
}

void Character::OnDamage(const DamageInfo& damageInfo)
{
	if (!mSvCanTakeDamage)
		return;

	if (mHealth.IsInvincible())
		return;

	// As soon as mDamageInfo is set, we must handle and reset it within the same frame
	// so that we can properly handle multi-frame damage
	ASSERT_MSG(!mDamageInfo.IsSet(), "DamageInfo not consumed since last set");

	mDamageInfo = damageInfo;

	// Only allow damage push vector in one direction (no diagonal push)
	//@TODO: If this is a general rule, we should probably put this right into DamageInfo
	// via a setter for the push vector.
	{
		Vector2I& pushVec = mDamageInfo.mPushVector;

		const uint16 x = MathEx::Abs(pushVec.x);
		const uint16 y = MathEx::Abs(pushVec.y);
		if (x > y)
		{
			pushVec.y = 0;
		}
		else if (y > x)
		{
			pushVec.x = 0;
		}
		else if (x == y)
		{
			// Favour direction we're facing
			const Vector2I& dirVec = GameHelpers::SpriteDirToUnitVector(GetSpriteDir());
			if (dirVec.x != 0)
			{
				pushVec.y = 0;
			}
			else
			{
				pushVec.x = 0;
			}
		}
	}
}
