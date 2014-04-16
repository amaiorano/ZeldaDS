#ifndef CHARACTER_STATE_H
#define CHARACTER_STATE_H

#include "gslib/Util/HsmUtil.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Anim/AnimControl.h"
#include "GameAnims.h"

// This file should only be included by hsm implementation cpps (not headers), so it's okay to do this
using namespace hsm;

struct CharacterSharedStateData : SharedStateData
{
	CharacterSharedStateData() 
		: mAttribCanTakeDamage(false)
	{
	}

	StateValue<bool> mAttribCanTakeDamage; // Can we take damage?
};

struct CharacterState : StateWithOwner<Character>
{
	virtual void PostAnimUpdate(GameTimeType deltaTime) { } 

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
