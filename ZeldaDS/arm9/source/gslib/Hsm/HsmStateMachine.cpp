#include "HsmStateMachine.h"

#define HSM_LOG(minLevel, numSpaces, printfArgs) \
	do \
	{ \
		if (m_debugLevel >= minLevel) \
		{ \
			HSM_PRINTF("HSM_%d: ", minLevel); \
			for (int i=0; i < (int)numSpaces; ++i) HSM_PRINTF(" "); /* Is there a better way to do this? */ \
			HSM_PRINTF printfArgs; \
		} \
	} while (false)

#ifdef NDEBUG
	#define HSM_LOG_TRANSITION(minLevel, depth, transType, pState)
#else
	#define HSM_LOG_TRANSITION(minLevel, depth, transTypeStr, pState) \
		HSM_LOG(minLevel, depth, ("%s: %s\n", transTypeStr, GetStateTypeId(*pState).name()))
#endif

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
	: m_pOwner(NULL)
	, m_pSharedStateData(NULL)
	, m_debugLevel(0)
{
}

StateMachine::~StateMachine()
{
	PopStatesToDepth(0);
	HSM_ASSERT(m_stateStack.empty());
	m_stateStack.clear();
	delete m_pSharedStateData;
}

void StateMachine::EvaluateStateTransitions(HsmTimeType deltaTime)
{
	HSM_ASSERT(m_pSharedStateData); // Make sure to set state data
	HSM_ASSERT(!m_stateStack.empty()); // Make sure to set an initial state!

	// After we make a transition, we must evaluate all transitions again until we get no transitions
	// from all states on the stack.
	bool keepEvaluating = true;
	size_t nextStartDepth = 0;
	while (keepEvaluating)
	{
		size_t startDepth = nextStartDepth;
		keepEvaluating = EvaluateStateTransitionsOnce(deltaTime, startDepth, nextStartDepth);
	}
}

void StateMachine::PerformStateActions(HsmTimeType deltaTime)
{
	VisitStatesOuterToInner(gCallPerformStateActionsVisitor, &deltaTime);	
}

State* StateMachine::FindState(StateTypeId stateType) const
{
	StateVector::const_iterator iter = m_stateStack.begin();
	const StateVector::const_iterator& iterEnd = m_stateStack.end();
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
	StateVector::iterator iter = m_stateStack.begin();
	const StateVector::iterator& iterEnd = m_stateStack.end();
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
	StateVector::reverse_iterator iter = m_stateStack.rbegin();
	const StateVector::reverse_iterator& iterEnd = m_stateStack.rend();
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
	if (depth >= m_stateStack.size())
		return NULL;

	return m_stateStack[depth];
}

void StateMachine::PushInitialState(State* pState)
{
	HSM_ASSERT(m_stateStack.empty());
	HSM_LOG_TRANSITION(1, 0, "Root", pState);
	m_stateStack.push_back(pState);
}

// Pops states from most inner up to and including depth
void StateMachine::PopStatesToDepth(size_t depth)
{
	const size_t numStatesToPop = m_stateStack.size() - depth;
	size_t rDepth = m_stateStack.size() - 1;

	for (size_t i = 0; i < numStatesToPop; ++i)
	{
		State* pStateToPop = m_stateStack[rDepth];
		HSM_LOG_TRANSITION(2, rDepth, "Pop", pStateToPop);
		DestroyState(pStateToPop);
		m_stateStack.pop_back(); //@todo: remove size()-depth elems after loop?
		--rDepth;
	}
}

// Returns true if a transition was made (and next depth to start at), meaning we must keep evaluating
bool StateMachine::EvaluateStateTransitionsOnce(HsmTimeType deltaTime, const size_t& startDepth, size_t& nextStartDepth)
{
	// Evaluate transitions from outer to inner states; if a valid sibling transition is returned,
	// we must pop inners up to and including the state that returned the transition, then push the 
	// new inner. If an inner transition is returned, we must pop inners up to but not including
	// the state that returned the transition (if any), then push the new inner.
	for (size_t depth = startDepth; depth < m_stateStack.size(); ++depth)
	{
		State* pCurrState = m_stateStack[depth];
		Transition& transition = pCurrState->EvaluateTransitions(deltaTime);

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
					if ( transition.GetStateType() == pInnerState->GetStateType() )
					{
						// Inner is already target state so keep going to next inner
					}
					else
					{
						// Pop all states under us and push target
						PopStatesToDepth(depth + 1);

						State* pTargetState = transition.CreateState(this);
						HSM_LOG_TRANSITION(1, depth + 1, "Inner", pTargetState);
						m_stateStack.push_back(pTargetState);
						
						nextStartDepth = depth + 1;
						return true;
					}
				}
				else
				{
					// No state under us so just push target
					State* pTargetState = transition.CreateState(this);
					HSM_LOG_TRANSITION(1, depth + 1, "Inner", pTargetState);
					m_stateStack.push_back(pTargetState);
					
					nextStartDepth = depth + 1;
					return true;
				}
			}
			break;

			case Transition::InnerEntry:
			{
				// If current state has no inner (is currently the innermost), then push the entry state
				if ( !GetStateAtDepth(depth + 1) )
				{
					State* pTargetState = transition.CreateState(this);
					HSM_LOG_TRANSITION(1, depth + 1, "Entry", pTargetState);
					m_stateStack.push_back(pTargetState);
					
					nextStartDepth = depth + 1;
					return true;
				}
			}
			break;

			case Transition::Sibling:
			{
				PopStatesToDepth(depth);

				State* pTargetState = transition.CreateState(this);
				HSM_LOG_TRANSITION(1, depth, "Sibling", pTargetState);
				m_stateStack.push_back(pTargetState);

				nextStartDepth = depth;
				return true;
			}
			break;

		} // end switch on transition type
	} // end for each depth

	return false;
}
