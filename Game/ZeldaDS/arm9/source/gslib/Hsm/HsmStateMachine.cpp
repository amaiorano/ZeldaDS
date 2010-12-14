#include "HsmStateMachine.h"

#define HSM_LOG(minLevel, numSpaces, printfArgs) \
	do \
	{ \
		if (mDebugLevel >= minLevel) \
		{ \
			HSM_PRINTF("HSM_%d: ", minLevel); \
			for (int i=0; i < (int)numSpaces; ++i) HSM_PRINTF(" "); /* Is there a better way to do this? */ \
			HSM_PRINTF printfArgs; \
		} \
	} while (false)

#ifdef HSM_DEBUG
	#define HSM_LOG_TRANSITION(minLevel, depth, transTypeStr, pState) \
		HSM_LOG(minLevel, depth, ("%-8s: %s\n", transTypeStr, GetStateDebugName(*pState)))
#else
	#define HSM_LOG_TRANSITION(minLevel, depth, transType, pState)
#endif

namespace HsmInternal
{
	// Friend function of State class so we can set private values on it
	void InitState(State* pState, StateMachine& stateMachine)
	{
		pState->mpOwnerStateMachine = &stateMachine;
	}

	State* CreateState(const Transition& transition, StateMachine& stateMachine)
	{
		State* pState = transition.GetStateFactory().AllocState();
		HsmInternal::InitState(pState, stateMachine);
		return pState;
	}

	void InvokeStateOnEnter(State* pState, const Transition& transition)
	{
		transition.GetStateFactory().InvokeStateOnEnter(pState, transition.GetStateArgs());
	}

	void DestroyState(State* pState)
	{
		delete pState;
	}

	void InvokeStateOnExit(State* pState)
	{
		pState->OnExit();
	}	
}

namespace
{
	struct CallPerformStateActionsVisitor : public StateVisitor
	{
		virtual bool OnVisit(State* pState, void* pUserData = NULL)
		{
			HsmTimeType& deltaTime = *static_cast<HsmTimeType*>(pUserData);
			pState->PerformStateActions( deltaTime );
			return true;
		}
	} gCallPerformStateActionsVisitor;

} // anonymous namespace


StateMachine::StateMachine()
	: mpOwner(NULL)
	, mpSharedStateData(NULL)
	, mDebugLevel(0)
{
}

StateMachine::~StateMachine()
{
	PopStatesToDepth(0);
	HSM_ASSERT(mStateStack.empty());
	mStateStack.clear();
	delete mpSharedStateData;
}

void StateMachine::EvaluateStateTransitions()
{
	HSM_ASSERT(mpSharedStateData); // Make sure to set state data
	HSM_ASSERT(!mStateStack.empty()); // Make sure to set an initial state!

	// After we make any transition, we must evaluate all transitions again from the top
	// until we get no transitions from all states on the stack.
	bool keepEvaluating = true;
	while (keepEvaluating)
	{
		keepEvaluating = EvaluateStateTransitionsOnce();
	}
}

void StateMachine::PerformStateActions(HsmTimeType deltaTime)
{
	VisitStatesOuterToInner(gCallPerformStateActionsVisitor, &deltaTime);
}

State* StateMachine::FindState(StateTypeId stateType) const
{
	StateStack::const_iterator iter = mStateStack.begin();
	const StateStack::const_iterator& iterEnd = mStateStack.end();
	for ( ; iter != iterEnd; ++iter)
	{
		State*const& pState = *iter;
		if (pState->GetStateType() == stateType)
			return pState;
	}
	return NULL;
}

void StateMachine::VisitStatesOuterToInner(StateVisitor& visitor, void* pUserData)
{
	StateStack::iterator iter = mStateStack.begin();
	const StateStack::iterator& iterEnd = mStateStack.end();
	for ( ; iter != iterEnd; ++iter)
	{
		State*& pState = *iter;
		const bool keepVisiting = visitor.OnVisit(pState, pUserData);
		if (!keepVisiting)
			return;
	}
}

void StateMachine::VisitStatesInnerToOuter(StateVisitor& visitor, void* pUserData)
{
	StateStack::reverse_iterator iter = mStateStack.rbegin();
	const StateStack::reverse_iterator& iterEnd = mStateStack.rend();
	for ( ; iter != iterEnd; ++iter)
	{
		State* pState = *iter;
		const bool keepVisiting = visitor.OnVisit(pState, pUserData);
		if (!keepVisiting)
			return;
	}
}

State* StateMachine::GetStateAtDepth(size_t depth)
{
	if (depth >= mStateStack.size())
		return NULL;

	return mStateStack[depth];
}

void StateMachine::CreateAndPushInitialState(const Transition& initialTransition)
{
	HSM_ASSERT(mStateStack.empty());

	State* pTargetState = HsmInternal::CreateState(initialTransition, *this);
	HSM_LOG_TRANSITION(1, 0, "Root", pTargetState);
	mStateStack.push_back(pTargetState);
	HsmInternal::InvokeStateOnEnter(pTargetState, initialTransition);
}

// Pops states from most inner up to and including depth
void StateMachine::PopStatesToDepth(size_t depth)
{
	const size_t numStatesToPop = mStateStack.size() - depth;
	size_t rDepth = mStateStack.size() - 1;

	for (size_t i = 0; i < numStatesToPop; ++i)
	{
		State* pStateToPop = mStateStack[rDepth];
		HSM_LOG_TRANSITION(2, rDepth, "Pop", pStateToPop);
		HsmInternal::InvokeStateOnExit(pStateToPop);
		HsmInternal::DestroyState(pStateToPop);
		mStateStack.pop_back(); //@todo: remove size()-depth elems after loop?
		--rDepth;
	}
}

// Returns true if a transition was made (and next depth to start at), meaning we must keep evaluating
bool StateMachine::EvaluateStateTransitionsOnce()
{
	// Evaluate transitions from outer to inner states; if a valid sibling transition is returned,
	// we must pop inners up to and including the state that returned the transition, then push the
	// new inner. If an inner transition is returned, we must pop inners up to but not including
	// the state that returned the transition (if any), then push the new inner.
	for (size_t depth = 0; depth < mStateStack.size(); ++depth)
	{
		State* pCurrState = mStateStack[depth];
		Transition transition = pCurrState->EvaluateTransitions();

		switch (transition.GetTransitionType())
		{
			case Transition::No:
			{
				continue;
			}
			break;

			case Transition::Inner:
			{
				if (State* pInnerState = GetStateAtDepth(depth + 1))
				{
					if ( transition.GetTargetStateType() == pInnerState->GetStateType() )
					{
						// Inner is already target state so keep going to next inner
					}
					else
					{
						// Pop all states under us and push target
						PopStatesToDepth(depth + 1);

						State* pTargetState = HsmInternal::CreateState(transition, *this);
						HSM_LOG_TRANSITION(1, depth + 1, "Inner", pTargetState);
						mStateStack.push_back(pTargetState);
						HsmInternal::InvokeStateOnEnter(pTargetState, transition);
						return true;
					}
				}
				else
				{
					// No state under us so just push target
					State* pTargetState = HsmInternal::CreateState(transition, *this);
					HSM_LOG_TRANSITION(1, depth + 1, "Inner", pTargetState);
					mStateStack.push_back(pTargetState);
					HsmInternal::InvokeStateOnEnter(pTargetState, transition);
					return true;
				}
			}
			break;

			case Transition::InnerEntry:
			{
				// If current state has no inner (is currently the innermost), then push the entry state
				if ( !GetStateAtDepth(depth + 1) )
				{
					State* pTargetState = HsmInternal::CreateState(transition, *this);
					HSM_LOG_TRANSITION(1, depth + 1, "Entry", pTargetState);
					mStateStack.push_back(pTargetState);
					HsmInternal::InvokeStateOnEnter(pTargetState, transition);
					return true;
				}
			}
			break;

			case Transition::Sibling:
			{
				PopStatesToDepth(depth);

				State* pTargetState = HsmInternal::CreateState(transition, *this);
				HSM_LOG_TRANSITION(1, depth, "Sibling", pTargetState);
				mStateStack.push_back(pTargetState);
				HsmInternal::InvokeStateOnEnter(pTargetState, transition);
				return true;
			}
			break;

		} // end switch on transition type
	} // end for each depth

	return false;
}
