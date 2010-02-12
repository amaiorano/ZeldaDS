#ifndef ACTOR_SHARED_STATE_DATA_H
#define ACTOR_SHARED_STATE_DATA_H

#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Hsm/HsmState.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Anim/AnimControl.h"
#include "GameAnims.h"

//@TODO: Rename this file and types - Actor is the base for any entity in the game. This file
// contains HSM stuff for characters (CharacterSharedStateData...)

struct ActorSharedStateData : SharedStateData
{
	ActorSharedStateData() 
		: mHealth(6)
	{
	}

	virtual ~ActorSharedStateData()
	{
	}

	virtual void PostHsmUpdate(HsmTimeType deltaTime)
	{
	}

	// Data...
	int16 mHealth; // Number of half-hearts
};

template <typename SharedStateDataChild, typename OwnerType>
struct ActorStateBase : ClientStateBase<SharedStateDataChild, OwnerType>
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
