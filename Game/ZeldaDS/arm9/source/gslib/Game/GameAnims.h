#ifndef GAME_ANIMS_H
#define GAME_ANIMS_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Generic.h"
#include "gslib/Core/Types.h"
#include "gslib/Anim/AnimAssetManager.h"

namespace SpriteDir
{
	// The order here matches the expected order of sprites in the tile maps
	enum Type
	{
		Right,
		Left,
		Down,
		Up,

		NumTypes,
		None = Down,	// Used for anims that are not direction-specific
		Unset = ~0
	};
}

namespace BaseAnim
{
	enum Type
	{
		// Character anims
		Spawn,
		Idle,
		Move,
		Attack,
		UseItem,
		Die,

		// Item anims
		ItemDefault,

		NumTypes,
		Unset = ~0
	};
}

namespace GameActor
{
	enum Type
	{
		None,		// Used for anims that are not gameActor-specific

		Hero,

		// Enemy list (see http://zelda.wikia.com/wiki/Darknut)

		// Grunts (non-boss enemies)
		Armos,		// Statues that come alive when you touch them
		BladeTrap,	// Magnetic spiky blocks
		Boulder,	// Come down from mountains
		Bubble,		// Skull in bubble, if player touches, can't use sword for a short time
		Darknut,	// Caped knight, impervious to frontal damage
		Gel,		// Tear-drop enemy, can be killed by boomerang, never drops items
		Ghini,		// Ghosts in cemetery
		Gibdo,		// Mummies
		Goriya,		// Boomerang throwing dudes
		Keese,		// Bat
		Lanmola,	// Caterpillar
		Leever,		// Blue/Red sand creatures (bury into ground, come up to attack)
		LikeLike,	// Tube-like creature that eats shields
		Lynel,		// Blue/Red centaurs
		Moblin,		// Orange/Blue bees that shoot arrows (blue ones can drop bombs)
		Moldorn,	// Giant worms
		Octorok,	// Blue/Red weird spiky thing that shoots rocks (first and easiest enemies)
		Peahat,		// Yellow flower creature that spins, only takes damage when stops spinning
		PolsVoice,	// Rabbit like creatures that jump around
		Stalfos,	// Skeleton
		Rope,		// Rope, charges player upon sight
		Tektite,	// Spiders that jump
		Vire,		// Blue bat-like demons that jump up and down
		WallMaster,	// Blue hands that bring hero back to start of dungeon
		WizzRobe,	// Hooded wizards that teleport
		Zol,		// Splits into Gels
		Zola,		// Water creature that fires projectiles at player

		// Bosses (@TODO: Get these names)
		Dragon,
		Rhino,
		Boss3,
		Boss4,
		Boss5,
		Boss6,
		Boss7,
		Boss8,
		Boss9,

		// Items / Weapons
		Sword,
		Boomerang, // Brown, Blue
		Bow,
		Arrow, // Brown, Silver
		Bomb,
		Candle, // Blue, Red
		Ladder,
		Raft,
		Staff,
		MagicBook,
		Key, // Normal, Master

		NumTypes,
	};

	const int NumGrunts = Zola - Armos + 1;
	const int NumBosses = Boss9 - Dragon + 1;
	const int NumEnemies = NumGrunts + NumBosses;
	const int NumItems = Key - Sword + 1;

} // namespace GameActor

// Macro to create AnimAssetKey
// AnimAssetKey is 32 bits: 2 bits for SpriteDir, 15 for BaseAnim and 15 for GameActor
#define MakeAnimAssetKey(GameActor, BaseAnim, SpriteDir) (AnimAssetKey)((uint32)SpriteDir << 30 | (uint32)BaseAnim << 15 | (uint32)GameActor)
CT_ASSERT_MSG((GameActor::NumTypes <= mpl::pow<2, 15>::value && BaseAnim::NumTypes <= mpl::pow<2, 15>::value && SpriteDir::NumTypes <= mpl::pow<2, 2>::value), Not_enough_bits_for_enum_range);

// Loads up all animations (create + add to AnimAssetManager)
void LoadAllGameAnimAssets();

#endif // GAME_ANIMS_H
