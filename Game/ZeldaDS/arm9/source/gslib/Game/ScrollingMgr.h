#ifndef SCROLLING_MGR_H
#define SCROLLING_MGR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Math/Vector2.h"
#include "hsm/StateMachine.h"

namespace ScrollDir
{
	enum Type { None, Right, Left, Up, Down };
}

class IScrollingEventListener
{
public:
	// Called by ScrollingMgr::StartScrolling() is called
	virtual void OnScrollingBegin() { }

	// Called by ScrollingMgr::Update() when scrolling has just ended
	virtual void OnScrollingEnd() { }
};

class ScrollingMgr : public Singleton<ScrollingMgr>
{
public:
	void Init();
	void Shutdown();

	void Reset(const Vector2I& startScreen);

	void Update(GameTimeType deltaTime);

	void AddEventListener(IScrollingEventListener* pListener);
	void RemoveEventListener(IScrollingEventListener* pListener);

	const Vector2I& GetCurrScreen() const { return mCurrScreen; }
	//@NOTE: Not exposing SetCurrScreen() as it should be set via Init() to ensure state consistency
	//void SetCurrScreen(const Vector2I& newScreen) { mCurrScreen = newScreen; }

	void StartScrolling(ScrollDir::Type scrollDir);
	bool IsScrolling() const;

private:
	friend class Singleton<ScrollingMgr>;
	ScrollingMgr();

	void UpdateCameraPos();
	
	void CallOnScrollingBegin();
	void CallOnScrollingEnd();

	hsm::StateMachine mStateMachine;

	friend struct ScrollingStates;
	// mCurrScreen and mScrollOffset together make up the camera's world position
	Vector2I mCurrScreen;
	Vector2I mScrollOffset;

	ScrollDir::Type mScrollDir;
	
	typedef std::vector<IScrollingEventListener*> EventListenerList;
	EventListenerList mEventListeners;
};

#endif // SCROLLING_MGR_H
