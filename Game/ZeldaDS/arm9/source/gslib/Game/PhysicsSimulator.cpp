#include "PhysicsSimulator.h"
#include "IPhysical.h"
#include "SceneGraph.h"
#include "WorldMap.h"
#include "Camera.h"
#include "Player.h"
#include "Enemy.h"
#include "Boomerang.h" //@TODO: REPLACE WITH "Weapon.h"
#include "DebugVars.h"

class PhysicsHelpers // This class is a friend of IPhysical
{
public:
	// Collision Handlers
	static void OnCollision(Player& player, Enemy& enemy, const Vector2I& lhsOffset)
	{
		// On collision, we move each one halfway of their respective offsets.
		//@NOTE: if vector is 1, this divide truncates value to 0
		//lhsOffset /= 2;
		//Vector2I rhsOffset = -lhsOffset;
		//
		//player.SetPosition(player.GetPosition() + lhsOffset);
		//enemy.SetPosition(enemy.GetPosition() + rhsOffset);

		static CollisionInfo collInfo; //@MT_UNSAFE
		collInfo.mpCollidingWith = &player;
		collInfo.mPushVector = -lhsOffset; // Enemy pushes Player
		enemy.OnCollision(collInfo);
	}

	static void OnCollision(Enemy& enemy1, Enemy& enemy2, const Vector2I& lhsOffset)
	{
		// On collision, we move each one halfway of their respective offsets.
		//lhsOffset /= 2;
		//Vector2I rhsOffset = -lhsOffset;
		//enemy1.SetPosition(enemy1.GetPosition() + lhsOffset);
		//enemy2.SetPosition(enemy2.GetPosition() + rhsOffset);
	}

	static void OnCollision(Weapon& weapon, Character& character, Vector2I lhsOffset)
	{
		static CollisionInfo collInfo; //@MT_UNSAFE
		collInfo.mpCollidingWith = &character;
		collInfo.mPushVector = lhsOffset; // Weapon pushes Character
		weapon.OnCollision(collInfo);
	}

	static void OnCollision(Weapon& weapon, Player& player, Vector2I lhsOffset)
	{
		if (!weapon.IsPlayerWeapon())
			OnCollision(weapon, static_cast<Character&>(player), lhsOffset);
	}

	static void OnCollision(Weapon& weapon, Enemy& enemy, Vector2I lhsOffset)
	{
		if (weapon.IsPlayerWeapon())
			OnCollision(weapon, static_cast<Character&>(enemy), lhsOffset);
	}

	// Helpers
	static void IntegratePhysical(IPhysical& physical, GameTimeType deltaTime)
	{
		// We integrate physical objects in world space only (so attached objects should not
		// set velocity or impulses!)
		if (physical.mVelocity.Length() > 0 || physical.mImpulse.Length() > 0)
		{
			const Vector2I& currPos = physical.GetPhysicalPosition();
			physical.SetPhysicalPosition(currPos + (physical.mVelocity * deltaTime) + physical.mImpulse);
			physical.mImpulse.Reset(InitZero);
		}
	}

	template <typename List>
	static void IntegrateEachPhysical(List& list, GameTimeType deltaTime)
	{
		typedef typename List::iterator Iterator;
		for (Iterator curr = list.begin(); curr != list.end(); ++curr)
		{
			IntegratePhysical(**curr, deltaTime);
		}
	}

	template <typename List1, typename List2>
	static void ActorActorCollisions(List1& list1, List2& list2)
	{
		typedef typename List1::iterator Iterator1;
		typedef typename List2::iterator Iterator2;

		for (Iterator1 lhsIter = list1.begin(); lhsIter != list1.end(); ++lhsIter)
		{
			IPhysical* pLhsPhysical = (*lhsIter);

			//@TODO: if collision against actors disabled for lhsIter, skip

			const BoundingBox& lhsBBox = pLhsPhysical->GetBoundingBox();

			for (Iterator2 rhsIter = list2.begin(); rhsIter != list2.end(); ++rhsIter)
			{
				IPhysical* pRhsPhysical = (*rhsIter);

				//@TODO: if collision against actors or against lhsIter type disabled for rhsIter, skip

				// Don't collide against oneself (happens when both lists are the same)
				if (pLhsPhysical == pRhsPhysical)
					continue;

				const BoundingBox& rhsBBox = pRhsPhysical->GetBoundingBox();
				
				Vector2I lhsOffset;
				if (lhsBBox.CollidesWith(rhsBBox, lhsOffset))
				{
					// Collision found! We purposely pass in the most derived type here
					// to call the proper overload of OnCollision().
					OnCollision(**lhsIter, **rhsIter, lhsOffset);
				}
			}
		}
	}

	template <typename List>
	static void ActorTileCollisions(List& list, WorldMap& worldMap)
	{
		typedef typename List::iterator Iterator;
	
		for (Iterator curr = list.begin(); curr != list.end(); ++curr)
		{
			IPhysical* pCurrPhysical = (*curr);

			//@TODO: if collision against world disabled for curr, skip

			BoundingBox actorBBox = pCurrPhysical->GetBoundingBox();
			const Vector2I& actorPos = pCurrPhysical->GetPhysicalPosition();

			Vector2I corners[] =
			{
				actorPos,
				actorPos + Vector2I(actorBBox.w, 0),
				actorPos + Vector2I(actorBBox.w, actorBBox.h),
				actorPos + Vector2I(0, actorBBox.h)
			};

			BoundingBox tileBBox;
			for (uint16 i = 0; i < NUM_ARRAY_ELEMS(corners); ++i)
			{
				if ( worldMap.GetTileBoundingBoxIfCollision(corners[i], tileBBox) )
				{
					Vector2I offset;
					if ( actorBBox.CollidesWith(tileBBox, offset) )
					{
						if ( !pCurrPhysical->AllowWorldOverlap() )
						{
							const Vector2I& currPos = pCurrPhysical->GetPhysicalPosition();
							pCurrPhysical->SetPhysicalPosition(currPos + offset);
						}

						static CollisionInfo collInfo; //@MT_UNSAFE
						collInfo.mpCollidingWith = NULL;
						collInfo.mPushVector = offset;
						pCurrPhysical->OnCollision(collInfo);

						// The other corners are now invalid (we've been moved)
						// so there's no point checking the rest of them.
						break;
					}
				}
			}
		}
	}

}; // class PhysicsHelpers


void PhysicsSimulator::IntegrateAndApplyCollisions(GameTimeType deltaTime)
{
	SceneGraph& sceneGraph = SceneGraph::Instance();
	WorldMap& worldMap = sceneGraph.GetWorldMap();
	PhysicalList& physicals = sceneGraph.GetPhysicalList();
	PlayerList& players = sceneGraph.GetPlayerList();
	EnemyList& enemies = sceneGraph.GetEnemyList();
	WeaponList& playerWeapons = sceneGraph.GetPlayerWeaponList();
	WeaponList& enemyWeapons = sceneGraph.GetEnemyWeaponList();

	if (deltaTime > 0)
	{
		// Move all IPhysical objects
		PhysicsHelpers::IntegrateEachPhysical(physicals, deltaTime);

		// Resolve game object collisions
		PhysicsHelpers::ActorActorCollisions(players, enemies);
		PhysicsHelpers::ActorActorCollisions(enemies, enemies);
		PhysicsHelpers::ActorActorCollisions(playerWeapons, enemies);
		PhysicsHelpers::ActorActorCollisions(enemyWeapons, players);
		//@TODO: Player-Items -> on collision, player picks up (rupees, bombs, treasure)

		// Resolve world collisions
		PhysicsHelpers::ActorTileCollisions(players, worldMap);
		PhysicsHelpers::ActorTileCollisions(enemies, worldMap);
		PhysicsHelpers::ActorTileCollisions(playerWeapons, worldMap);
		PhysicsHelpers::ActorTileCollisions(enemyWeapons, worldMap);
	}

#if DEBUG_VARS_ENABLED
	extern void DrawQuad(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color, uint16 alpha);

	if ( DEBUG_VAR_GET(DrawCollisionBounds) )
	{
		const Camera& camera = Camera::Instance();

		PhysicalList::iterator iter = physicals.begin();
		for ( ; iter != physicals.end(); ++iter)
		{
			BoundingBox bbox = (*iter)->GetBoundingBox();
			bbox.pos = camera.WorldToScreen(bbox.pos);
			DrawQuad(bbox.pos.x, bbox.pos.y, bbox.w, bbox.h, RGB15(255,0,0), 15);
		}

		const Vector2I camPos = camera.GetPosition();
		BoundingBox bbox;

		for (uint16 numTilesY = 0; numTilesY < GameNumScreenMetaTilesY; ++numTilesY)
		{
			for (uint16 numTilesX = 0; numTilesX < GameNumScreenMetaTilesX; ++numTilesX)
			{
				Vector2I currTilePos(camPos.x + numTilesX * GameMetaTileSizeX, camPos.y + numTilesY * GameMetaTileSizeX);

				if ( worldMap.GetTileBoundingBoxIfCollision(currTilePos, bbox) )
				{
					bbox.pos = camera.WorldToScreen(bbox.pos);
					DrawQuad(bbox.pos.x, bbox.pos.y, bbox.w, bbox.h, RGB15(255,0,0), 15);
				}
			}
		}
	}
#endif // DEBUG_DRAW_ENABLED

}
