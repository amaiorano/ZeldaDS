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

		// See www.gamefaqs.com/console/nes/file/563433/23898

		// Overworld
		
		Armos,		// Statues that come alive when you touch them
		Ghini,		// Ghosts in cemetery
		Leever,		// Blue/Red sand creatures (bury into ground, come up to attack)
		Lynel,		// Blue/Red centaurs
		Moblin,		// Orange/Blue bees that shoot arrows (blue ones can drop bombs)
		Octorok,	// Blue/Red weird spiky thing that shoots rocks (first and easiest enemies)
		Peahat,		// Yellow flower creature that spins, only takes damage when stops spinning
		Rock,		// Falling rock
		Tektite,	// Spiders that jump
		Zola,		// Water creature that fires projectiles at player

		// Underworld
		Snake,		//@TODO: Wrong name
		Knight,		//@TODO: Wrong name
		Skeleton,	//@TODO: Wrong name
		Goriyas,	// Boomerang throwing dudes
		Rhino,		//@TODO: Wrong name


		// Bosses
		//...

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
}

/*
== Weapons ==

Normal:
* Sword (Wood/Silver/Master)
* Boomerang (Brown/Blue)
* Bomb
* Candle (Blue/Red Flame)

Projectiles:
* Sword => Flashing Sword
* Bow => Arrows
* Staff => Sonic Wave
* Staff + Book => Sonic Wave to Flame

*/



// Macro to create AnimAssetKey
// AnimAssetKey is 32 bits: 2 bits for SpriteDir, 15 for BaseAnim and 15 for GameActor
#define MakeAnimAssetKey(GameActor, BaseAnim, SpriteDir) (AnimAssetKey)((uint32)SpriteDir << 30 | (uint32)BaseAnim << 15 | (uint32)GameActor)
CT_ASSERT_MSG((GameActor::NumTypes <= mpl::pow<2, 15>::value && BaseAnim::NumTypes <= mpl::pow<2, 15>::value && SpriteDir::NumTypes <= mpl::pow<2, 2>::value), Not_enough_bits_for_enum_range);

// Loads up all animations (create + add to AnimAssetManager)
void LoadAllGameAnimAssets();

#endif // GAME_ANIMS_H
