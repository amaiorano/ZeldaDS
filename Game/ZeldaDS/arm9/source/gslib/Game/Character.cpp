#include "Character.h"
#include "ActorSharedStateData.h"

void Character::Init(const Vector2I& initPos)
{
	mInitPos = initPos;
	
	InitStateMachine();
	mpSharedStateData = static_cast<CharacterSharedStateData*>(&mStateMachine.GetSharedStateData());
	ASSERT(mpSharedStateData); // If invalid, child class didn't init state machine properly
}

void Character::OnAddToScene()
{
	Base::OnAddToScene();

	SetPosition(mInitPos);
	mDamageInfo.Reset();
	PlayAnim(BaseAnim::Idle); // Set initial pose
}

void Character::Update(GameTimeType deltaTime)
{
	if (deltaTime > 0) // Total cop out, we don't handle deltaTime == 0 very well
	{
		// Character's must set their velocity every frame to move
		SetVelocity(InitZero);

		mStateMachine.Update(deltaTime);
		mHealth.Update(deltaTime);
	}

	Base::Update(deltaTime);
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
}
