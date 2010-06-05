#ifndef ACTOR_SHARED_STATE_DATA_H
#define ACTOR_SHARED_STATE_DATA_H

#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Hsm/HsmState.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Anim/AnimControl.h"
#include "GameAnims.h"

//@TODO: Rename this file: CharacterState.h?

struct CharacterSharedStateData : SharedStateData
{
	CharacterSharedStateData() 
		: mAttribCanTakeDamage(false)
	{
	}

	Attribute<bool> mAttribCanTakeDamage; // Can we take damage?
};

template <typename SharedStateDataChild, typename OwnerType>
struct CharacterStateBase : ClientStateBase<SharedStateDataChild, OwnerType>
{
	void PlayAnim(BaseAnim::Type anim)
	{
		this->Owner().PlayAnim(anim);
	}

	void PlayGlobalAnim(BaseAnim::Type anim)
	{
		this->Owner().PlayGlobalAnim(anim);
	}

	bool IsAnimFinished()
	{
		return this->Owner().IsAnimFinished();
	}
};

#endif // ACTOR_SHARED_STATE_DATA_H
