#ifndef SCROLLING_MGR_H
#define SCROLLING_MGR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Math/Vector2.h"

namespace ScrollDir
{
	enum Type { None, Right, Left, Up, Down };
}

class ScrollingMgr : public Singleton<ScrollingMgr>
{
public:
	void Init(const Vector2I& startScreen);
	void Update(GameTimeType deltaTime);

	const Vector2I& GetCurrScreen() { return mCurrScreen; }
	//@NOTE: Not exposing SetCurrScreen() as it should be set via Init() to ensure state consistency
	//void SetCurrScreen(const Vector2I& newScreen) { mCurrScreen = newScreen; }

	void StartScrolling(ScrollDir::Type scrollDir);
	bool IsScrolling() const;

private:
	friend class Singleton<ScrollingMgr>;
	ScrollingMgr() { }

	void UpdateCameraPos();

	StateMachine mStateMachine;
	SharedStateData* mpSharedStateData; // Alias for convenience

	friend struct ScrollingStates;
	// mCurrScreen and mScrollOffset together make up the camera's world position
	Vector2I mCurrScreen;
	Vector2I mScrollOffset;

	ScrollDir::Type mScrollDir;
};

#endif // SCROLLING_MGR_H
