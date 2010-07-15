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

// This class sits between StateT and State, allowing us to add functions
// that must be in a non-template class (i.e. for visitor functors)
struct CharacterStateExt : public State
{
	virtual void PostAnimUpdate(HsmTimeType deltaTime) { } 
};

template <typename SharedStateDataChild, typename OwnerType>
struct CharacterState : StateT<SharedStateDataChild, OwnerType, CharacterStateExt>
{
	typedef StateT<SharedStateDataChild, OwnerType, CharacterStateExt> Base;
	
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
