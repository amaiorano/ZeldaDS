#ifndef HSM_STATEMACHINE_H
#define HSM_STATEMACHINE_H

/*
TODO:
- Dynamic transition objects (for deferred state)
- State construction args
- Deferred state transitions
- Move internal stuff to an Internal namespace
- Memory managment (client-provided Alloc/Free for memory into which we in-place new our objects)
- State* State::GetImmediateInnerState() (faster than using GetState() within states for certain idioms)

DONE:
- State* FindState(), IsInState(StateTypeId)
- InnerEntryTransition()
- Attrbitues

NOTES:
- Using OnEnter/OnExit instead of constructor/destructor for States so that clients don't have to
  declare a non-trivial constructor, which would be necessary to pass the pStateMachine up the chain.
  Downside is that clients may have to chain OnEnter()/OnExit() manually to their base (not outer) state.

*/
// HSM Data:
// - Shared data across all states (StateSharedData)
//		- Normal data
//		- Attributes (pushed in OnEntry, popped when state destructs)
// - State-specific data (data members of state)
// - State constructor args (TODO)

#include "HsmTransition.h"

// The state machine
class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	// Initialization functions

	// Optional: sets an owner object that can be retrieved via Owner() from within states
	// that derive from ClientStateBase.
	void SetOwner(void* pOwner)
	{
		HSM_ASSERT( !m_pOwner && pOwner );
		m_pOwner = pOwner;
	}

	// Required: sets shared state data (transfers ownership)
	void SetSharedStateData(SharedStateData* pNewSharedStateData)
	{
		HSM_ASSERT( !m_pSharedStateData && pNewSharedStateData );
		m_pSharedStateData = pNewSharedStateData;
	}
	
	// Required: sets the state to start in
	template <typename ChildState>
	void SetInitialState()
	{
		HSM_ASSERT( m_stateStack.empty() );
		State* pInitialState = SiblingTransition<ChildState>().CreateState(this);
		PushInitialState(pInitialState);
	}

	// Update function (call once per frame)
	void Update(HsmTimeType deltaTime)
	{
		EvaluateStateTransitions(deltaTime);
		PerformStateActions(deltaTime);
	}
	
	// Called by Update()
	void EvaluateStateTransitions(HsmTimeType deltaTime);
	void PerformStateActions(HsmTimeType deltaTime);

	// Accessors

	void* GetOwner()
	{
		return m_pOwner;
	}

	SharedStateData& GetSharedStateData()
	{
		HSM_ASSERT(m_pSharedStateData);
		return *m_pSharedStateData;
	}

	State* FindState(StateTypeId stateType) const;

	template <typename ChildState>
	bool IsInState() const
	{
		return FindState(GetStateTypeId(ChildState)) != NULL;
	}

	// Visitor functions

	void VisitStatesOuterToInner(StateVisitor& visitor, void* pUserData = NULL);
	void VisitStatesInnerToOuter(StateVisitor& visitor, void* pUserData = NULL);

	void SetDebugLevel(int debugLevel) { m_debugLevel = debugLevel; }

private:
	typedef std::vector<int>::size_type size_t;

	State* GetStateAtDepth(size_t depth);	

	void PushInitialState(State* pState);

	// Pops states from most inner up to and including depth
	void PopStatesToDepth(size_t depth);	

	// Returns true if a transition was made (and next depth to start at), meaning we must keep evaluating
	bool EvaluateStateTransitionsOnce(HsmTimeType deltaTime, const size_t& startDepth, size_t& nextStartDepth);	

private:
	void* m_pOwner; // Provided by client, accessed within states via Owner()
	SharedStateData* m_pSharedStateData; // Owned/deleted by state machine

	typedef std::vector<State*> StateVector;
	StateVector m_stateStack;

	int m_debugLevel;
};


// Inline State member fucntion definitions - defined here because they depend on certain types
// to be defined (StateMachine, Transitions, etc.)
inline Transition& State::EvaluateTransitions(HsmTimeType deltaTime)
{
	return NoTransition();
}

template <typename ChildState>
inline ChildState* State::FindState()
{
	return static_cast<ChildState*>( GetStateMachine().FindState( GetStateTypeId(ChildState) ) );
}

template <typename ChildState>
inline bool State::IsInState()
{
	return GetStateMachine().IsInState<ChildState>();
}



#endif // HSM_STATEMACHINE_H
