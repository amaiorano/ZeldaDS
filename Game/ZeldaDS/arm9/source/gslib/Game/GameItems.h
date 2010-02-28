#ifndef GAME_ITEMS_H
#define GAME_ITEMS_H

#include "GameAnims.h"
#include "gslib/Anim/AnimClientTypes.h"

//@TODO: Rename to GameItemAnims?
namespace GameItem
{
	enum Type
	{
		Sword,
		Boomerang,
	};
}

AnimPoseType GetGameItemPose(GameItem::Type gameItem, SpriteDir::Type dir);

#endif // GAME_ITEMS_H
