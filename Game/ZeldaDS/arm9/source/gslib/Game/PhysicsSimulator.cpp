#include "PhysicsSimulator.h"
#include "IPhysical.h"
#include "SceneGraph.h"
#include "WorldMap.h"
#include "Player.h"
#include "Enemy.h"
#include "Boomerang.h" //@TODO: REPLACE WITH "Weapon.h"

class PhysicsHelpers // This class is a friend of IPhysical
{
public:
	// Collision Handlers
	static void OnCollision(Player& player, Enemy& enemy, /*const Vector2I&*/Vector2I lhsOffset)
	{
		//IDEA: when a collision occurs, one entity is told and is reponsible for perhaps affecting
		// the other entity (i.e. Enemy hurts Player, Boomerang hurts/stuns Actor, Item is picked up by Player)
		// - Enemy::OnCollision(Player& player) -> calls Player::OnDamage()
		// - Item::OnCollision(Player& player) -> calls Player::OnCollect()
		// - CollisionInfo struct, DamageInfo struct, CollectInfo struct?
		// - Should OnCollision() be a virtual on IPhysical? Requires that type be known (RTTI?)

		// On collision, we move each one halfway of their respective offsets.
		//@NOTE: if vector is 1, this divide truncates value to 0
		//lhsOffset /= 2;
		//Vector2I rhsOffset = -lhsOffset;
		//
		//player.ModifyPosition() += lhsOffset;
		//enemy.ModifyPosition() += rhsOffset;

		static CollisionInfo collInfo; //@MT_UNSAFE
		collInfo.mpCollidingWith = &player;
		collInfo.mPushVector = -lhsOffset; // Collider pushes collidee
		enemy.OnCollision(collInfo);
	}

	static void OnCollision(Enemy& enemy1, Enemy& enemy2, const Vector2I& lhsOffset)
	{
		//enemy1.ModifyPosition() += offset;
	}

	static void OnCollision(Weapon& playerWeapon, Enemy& enemy, Vector2I lhsOffset)
	{
		static CollisionInfo collInfo; //@MT_UNSAFE
		collInfo.mpCollidingWith = &enemy;
		collInfo.mPushVector = -lhsOffset; // Collider pushes collidee
		playerWeapon.OnCollision(collInfo);
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

	// Move all IPhysical objects
	PhysicsHelpers::IntegrateEachPhysical(physicals, deltaTime);

	// Resolve object collisions
	PhysicsHelpers::ActorActorCollisions(players, enemies);
	PhysicsHelpers::ActorActorCollisions(enemies, enemies);

	//@TODO: Player-Items -> on collision, player picks up (rupees, bombs, treasure)
	//@TODO: Player-EnemyWeapon -> on collision, player is damaged, weapon stops doing damage and (eventually) dissapears

	PhysicsHelpers::ActorActorCollisions(playerWeapons, enemies);

	// Resolve world collisions
	PhysicsHelpers::ActorTileCollisions(players, worldMap);
	PhysicsHelpers::ActorTileCollisions(enemies, worldMap);
	PhysicsHelpers::ActorTileCollisions(playerWeapons, worldMap);
}