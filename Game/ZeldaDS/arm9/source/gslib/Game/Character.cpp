#include "Character.h"
#include "CharacterState.h"
#include "GameHelpers.h"

void Character::Init(const Vector2I& initPos)
{
	mInitPos = initPos;
	
	InitStateMachine();
	mpSharedStateData = static_cast<CharacterSharedStateData*>(&mStateMachine.GetSharedStateData());
	ASSERT(mpSharedStateData); // If invalid, child class didn't init state machine properly
}

void Character::InitStateMachine()
{
	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(CreateSharedStateData());
	mpSharedStateData = static_cast<CharacterSharedStateData*>(&mStateMachine.GetSharedStateData());

	// No initial state set - child must set one...
}

SharedStateData* Character::CreateSharedStateData()
{
	return new CharacterSharedStateData();
}

void Character::OnAddToScene()
{
	Base::OnAddToScene();

	SetPosition(mInitPos);
	mDamageInfo.Reset();
	PlayAnim(BaseAnim::Idle); // Set initial pose
	SetSpriteDir(SpriteDir::Down); // Set initial direction
}

struct CallPostAnimUpdateVisitor : public StateVisitor
{
	virtual bool OnVisit(State* pState, void* pUserData = NULL)
	{
		HsmTimeType& deltaTime = *static_cast<HsmTimeType*>(pUserData);
		static_cast<CharacterState*>(pState)->PostAnimUpdate( deltaTime );
		return true;
	}
} gCallPostAnimUpdateVisitor;

void Character::Update(GameTimeType deltaTime)
{
	if (deltaTime > 0) // Total cop out, we don't handle deltaTime == 0 very well
	{
		// Character's must set their velocity every frame to move
		SetVelocity(InitZero);

		mStateMachine.Update(deltaTime);
		mHealth.Update(deltaTime);
	}

	Base::Update(deltaTime); // Updates anim and render state
	mStateMachine.VisitStatesOuterToInner(gCallPostAnimUpdateVisitor, &deltaTime);
}

void Character::OnDamage(const DamageInfo& damageInfo)
{
	if (!mpSharedStateData->mAttribCanTakeDamage)
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
