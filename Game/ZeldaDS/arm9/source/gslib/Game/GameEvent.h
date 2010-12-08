#ifndef GAME_EVENT
#define GAME_EVENT

#include "gslib/Core/Core.h"
#include <string>

namespace GameEventType
{
	// Values correspond to ids in events.xml for Zelous
	enum Type
	{
		Warp = 1,
	};
};

struct GameEvent
{
	GameEvent(GameEventType::Type type, const Vector2I& tilePos)
		: mType(type)
		, mTilePos(tilePos)
	{
	}

	virtual ~GameEvent()
	{
	}

	GameEventType::Type mType;
	Vector2I mTilePos;
};

struct WarpGameEvent : public GameEvent
{
	typedef GameEvent Base;

	WarpGameEvent(const Vector2I& tilePos, const std::string& targetWorldMap, const Vector2I& targetTilePos)
		: Base(GameEventType::Warp, tilePos)
		, mTargetWorldMap(targetWorldMap)
		, mTargetTilePos(targetTilePos)
	{
	}

	std::string mTargetWorldMap;
	Vector2I mTargetTilePos;
};

#endif // GAME_EVENT
