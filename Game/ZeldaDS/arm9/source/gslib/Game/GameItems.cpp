#include "GameItems.h"
#include "data/items.h"

AnimPoseType GetGameItemPose(GameItem::Type gameItem, SpriteDir::Type dir)
{
	const uint16 tileIndex = (int)gameItem + (int)dir;
	const uint16 frameSize = 16*16;
	AnimPoseType pose = { (uint8*)itemsTiles + (frameSize * tileIndex), frameSize };
	return pose;
}
