#include "GameItems.h"
#include "gslib/Game/GameResources.h"

AnimPoseType GetGameItemPose(GameItem::Type gameItem, SpriteDir::Type dir)
{
	//@NOTE: This is fine as long as the item tiles never change
	static const uint8* pItemTiles = ResourceMgr::Instance().GetResource(GameResource::Gfx_Items).Data();

	const uint16 tileIndex = (int)gameItem + (int)dir;
	const uint16 frameSize = 16*16;
	return AnimPoseType((uint8*)pItemTiles + (frameSize * tileIndex), frameSize);
}
