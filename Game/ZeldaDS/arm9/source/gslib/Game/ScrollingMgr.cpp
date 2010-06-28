#include "ScrollingMgr.h"
#include "WorldMap.h"
#include "Camera.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"

struct ScrollingStates
{
	struct ScrollingSharedStateData : SharedStateData
	{
	};

	typedef ClientStateBase<ScrollingSharedStateData, ScrollingMgr> ScrollingStateBase;
	
	struct Root : ScrollingStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<NotScrolling>();
		}
	};

	struct NotScrolling : ScrollingStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (Owner().mScrollDir != ScrollDir::None)
			{
				return SiblingTransition<Scrolling>();
			}
			else
			{
				// Keep redrawing current screen's animated tiles
				WorldMap::Instance().DrawScreenTiles(Owner().mCurrScreen, Vector2I(0,0), DrawScreenTilesMode::AnimatedOnly);
			}

			return NoTransition();
		}
	};

	struct Scrolling : ScrollingStateBase
	{
		uint16 mScrollX;
		uint16 mScrollY;

		Scrolling()
			: mScrollX(0)
			, mScrollY(0)
		{		
		}

		virtual void OnExit()
		{
			Owner().mScrollOffset.Reset(InitZero);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (IsInState<Scrolling_Done>())
			{
				return SiblingTransition<NotScrolling>();
			}

			return InnerEntryTransition<Scrolling_PreScroll>();
		}

		void UpdateScrollState()
		{
			GraphicsEngine::GetBgLayer(2).SetScroll(mScrollX, mScrollY);
			GraphicsEngine::GetBgLayer(3).SetScroll(mScrollX, mScrollY);
			
			Owner().mScrollOffset.Reset(mScrollX, mScrollY);
			if (Owner().mScrollDir == ScrollDir::Left)
			{
				Owner().mScrollOffset.x -= HwScreenSizeX;
			}
			if (Owner().mScrollDir == ScrollDir::Up)
			{
				Owner().mScrollOffset.y -= HwScreenSizeY;
			}

			Owner().UpdateCameraPos();
		}
	};

	struct Scrolling_PreScroll : ScrollingStateBase
	{
		virtual void OnEnter()
		{
			Scrolling* pScrollingState = GetState<Scrolling>();
			ASSERT(pScrollingState->mScrollX == 0 && pScrollingState->mScrollY == 0);
			WorldMap& worldMap = WorldMap::Instance();
			const Vector2I& currScreen = Owner().mCurrScreen;

			switch (Owner().mScrollDir)
			{
			case ScrollDir::Right:
				worldMap.DrawScreenTiles(currScreen + Vector2I(1,0), Vector2I(1,0));
				break;

			case ScrollDir::Left:
				worldMap.DrawScreenTiles(currScreen, Vector2I(1,0));
				pScrollingState->mScrollX = HwScreenSizeX;
				pScrollingState->UpdateScrollState();

				//@HACK: To avoid tearing, wait until our scroll position has been updated before
				// drawing over current screen
				GraphicsEngine::WaitForVBlankAndPostVBlankUpdate();

				worldMap.DrawScreenTiles(currScreen + Vector2I(-1,0), Vector2I(0,0));
				break;

			case ScrollDir::Up:
				worldMap.DrawScreenTiles(currScreen, Vector2I(0,1));
				pScrollingState->mScrollY = HwScreenSizeY;
				pScrollingState->UpdateScrollState();
				
				//@HACK: Same as above
				GraphicsEngine::WaitForVBlankAndPostVBlankUpdate();

				worldMap.DrawScreenTiles(currScreen + Vector2I(0,-1), Vector2I(0,0));
				break;

			case ScrollDir::Down:
				worldMap.DrawScreenTiles(currScreen + Vector2I(0,1), Vector2I(0,1));
				break;

			case ScrollDir::None:
			default:
				FAIL();
			}
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<Scrolling_Scroll>();
		}
	};

	struct Scrolling_Scroll : ScrollingStateBase
	{
		bool mIsDoneScrolling;

		virtual void OnEnter()
		{
			mIsDoneScrolling = false;
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (mIsDoneScrolling)
			{
				return SiblingTransition<Scrolling_PostScroll>();
			}
			return NoTransition();
		}
		

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			Scrolling* pScrollingState = GetState<Scrolling>();
			const uint16 scrollDelta = 4 * deltaTime;

			switch (Owner().mScrollDir)
			{
			case ScrollDir::Right:
				pScrollingState->mScrollX += scrollDelta;
				pScrollingState->mScrollX = MathEx::Clamp(pScrollingState->mScrollX, 0, HwScreenSizeX);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollX >= HwScreenSizeX);
				break;

			case ScrollDir::Left:
				pScrollingState->mScrollX -= scrollDelta;
				pScrollingState->mScrollX = MathEx::Clamp(pScrollingState->mScrollX, 0, HwScreenSizeX);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollX <= 0);
				break;

			case ScrollDir::Up:
				pScrollingState->mScrollY -= scrollDelta;
				pScrollingState->mScrollY = MathEx::Clamp(pScrollingState->mScrollY, 0, HwScreenSizeY);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollY <= 0);
				break;

			case ScrollDir::Down:
				pScrollingState->mScrollY += scrollDelta;
				pScrollingState->mScrollY = MathEx::Clamp(pScrollingState->mScrollY, 0, HwScreenSizeY);
				pScrollingState->UpdateScrollState();
				mIsDoneScrolling = (pScrollingState->mScrollY >= HwScreenSizeY);
				break;

			case ScrollDir::None:
			default:
				FAIL();
			}
		}
	};

	struct Scrolling_PostScroll : ScrollingStateBase
	{
		virtual void OnEnter()
		{
			Scrolling* pScrollingState = GetState<Scrolling>();

			switch (Owner().mScrollDir)
			{
			case ScrollDir::Right:
				Owner().mCurrScreen += Vector2I(1, 0);
				break;

			case ScrollDir::Left:
				Owner().mCurrScreen += Vector2I(-1, 0);
				break;

			case ScrollDir::Up:
				Owner().mCurrScreen += Vector2I(0, -1);
				break;

			case ScrollDir::Down:
				Owner().mCurrScreen += Vector2I(0, 1);
				break;

			case ScrollDir::None:
			default:
				FAIL();
			}

			// Copy new current screen to 0,0 and reset scroll vars
			WorldMap::Instance().DrawScreenTiles(Owner().mCurrScreen, Vector2I(0,0));
			Owner().mScrollDir = ScrollDir::None; // Important to set this before UpdateScrollState() for offset to make sense
			pScrollingState->mScrollX = 0;
			pScrollingState->mScrollY = 0;
			pScrollingState->UpdateScrollState();
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<Scrolling_Done>();
		}
	};

	struct Scrolling_Done : ScrollingStateBase
	{
		virtual void OnEnter()
		{
			Owner().CallOnScrollingEnd();
		}
	};

};

ScrollingMgr::ScrollingMgr()
	: mpSharedStateData(0)
	, mCurrScreen(InitZero)
	, mScrollOffset(InitZero)
	, mScrollDir(ScrollDir::None)
{
}

void ScrollingMgr::Init(const Vector2I& startScreen)
{
	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(new ScrollingStates::ScrollingSharedStateData());
	mStateMachine.SetInitialState<ScrollingStates::Root>();

	mCurrScreen.Reset(startScreen);
	mScrollOffset.Reset(InitZero);
	UpdateCameraPos();

	mScrollDir = ScrollDir::None;

	// Draw the initial screen
	WorldMap::Instance().DrawScreenTiles(mCurrScreen, Vector2I(0,0));
}

void ScrollingMgr::Update(GameTimeType deltaTime)
{
	mStateMachine.Update(deltaTime);
}

void ScrollingMgr::AddEventListener(IScrollingEventListener* pListener)
{
	mEventListeners.push_back(pListener);
}

void ScrollingMgr::RemoveEventListener(IScrollingEventListener* pListener)
{
	EventListenerList::iterator iter = std::find(mEventListeners.begin(), mEventListeners.end(), pListener);
	ASSERT(iter != mEventListeners.end());
	mEventListeners.erase(iter);
}

void ScrollingMgr::StartScrolling(ScrollDir::Type scrollDir)
{
	ASSERT(mScrollDir == ScrollDir::None); // Don't start scrolling while already scrolling
	ASSERT(scrollDir != ScrollDir::None);
	mScrollDir = scrollDir;
	CallOnScrollingBegin();
}

bool ScrollingMgr::IsScrolling() const
{
	return (mScrollDir != ScrollDir::None);
}

void ScrollingMgr::UpdateCameraPos()
{
	const Vector2I HwScreenSize(HwScreenSizeX, HwScreenSizeY);

	const Vector2I& newCameraPos = ScaleComponents(mCurrScreen, HwScreenSize) + mScrollOffset;
	Camera::Instance().SetPosition(newCameraPos);
}

void ScrollingMgr::CallOnScrollingBegin()
{
	EventListenerList::iterator iter = mEventListeners.begin();
	for ( ; iter != mEventListeners.end(); ++iter)
		(*iter)->OnScrollingBegin();
}

void ScrollingMgr::CallOnScrollingEnd()
{
	EventListenerList::iterator iter = mEventListeners.begin();
	for ( ; iter != mEventListeners.end(); ++iter)
		(*iter)->OnScrollingEnd();
}
