#ifndef CHARACTER_STATE_H
#define CHARACTER_STATE_H

#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Hsm/HsmState.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Anim/AnimControl.h"
#include "GameAnims.h"

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
	typedef ClientStateBase<SharedStateDataChild, OwnerType> Base;
	
	// Bring in non-template dependent names that the compiler cannot deduce
	using Base::Owner;

	void PlayAnim(BaseAnim::Type anim)
	{
		Owner().PlayAnim(anim);
	}

	void PlayGlobalAnim(BaseAnim::Type anim)
	{
		Owner().PlayGlobalAnim(anim);
	}

	bool IsAnimFinished()
	{
		return Owner().IsAnimFinished();
	}
};

#endif // CHARACTER_STATE_H
