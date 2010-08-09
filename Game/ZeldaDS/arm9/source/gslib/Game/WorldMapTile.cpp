#include "WorldMapTile.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Hw/BackgroundLayer.h"

WorldMapTile::WorldMapTile(GameTileLayer::Type tileLayer, const Vector2I& worldTilePos)
{
	const BackgroundLayer& bgLayer = WorldMap::Instance().GetBgLayer(tileLayer);
	const uint8* pFrameGfx = bgLayer.GetTilesImagePtr();
	const uint16 tileIdx = WorldMap::Instance().GetTileIndex(tileLayer, worldTilePos);

	mAnimPose.frameSize = 16 * 16; // Fixed size of world map tiles
	mAnimPose.pFrameGfx = pFrameGfx + mAnimPose.frameSize * tileIdx;
}

void WorldMapTile::OnAddToScene()
{
	Base::OnAddToScene();

	// No anim to play, just set the final pose
	//GetAnimControl().PlayAnim( MakeAnimAssetKey(GameActor::Boomerang, BaseAnim::ItemDefault, SpriteDir::None));
	mpSprite->SetAnimPose(mAnimPose);
}

void WorldMapTile::Update(GameTimeType deltaTime)
{
	//@HACK: Don't call base class Update() because we don't want it to update animation
	// as we don't set one! Should probably set a flag on GameObject like mPlayAnim or something.
	//Base::Update(deltaTime);
}
