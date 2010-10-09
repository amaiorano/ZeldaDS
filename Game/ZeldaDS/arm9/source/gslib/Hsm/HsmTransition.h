#ifndef HSM_TRANSITION_H
#define HSM_TRANSITION_H

#include "HsmState.h"

struct StateFactory;

// Returns the one StateFactory instance for the input state
template <typename TargetState>
StateFactory& GetStateFactory();

// StateFactory is responsible for creating State instances
struct StateFactory
{
	virtual StateTypeId GetStateType() const = 0;
	virtual State* CreateState(StateMachine* pOwnerStateMachine) const = 0;
};

template <typename TargetState>
struct ConcreteStateFactory : StateFactory
{
	virtual StateTypeId GetStateType() const { return GetStateTypeId(TargetState); }

	virtual State* CreateState(StateMachine* pOwnerStateMachine) const
	{
		return ::CreateState<TargetState>(pOwnerStateMachine);
	}

private:
	friend StateFactory& GetStateFactory<TargetState>();
	ConcreteStateFactory() { }
};

template <typename TargetState>
StateFactory& GetStateFactory()
{
	static ConcreteStateFactory<TargetState> instance;
	return instance;
}

// Transitions are returned by EvaluateTransition(), and are most often constructed via
// the generator functions below.
struct Transition
{
	enum Type { Sibling, Inner, InnerEntry, No };

	// Defaults to No transition
	Transition()
		: mTransitionType(Transition::No)
		, mpStateFactory(0)
	{
	}

	Transition(Transition::Type transitionType, StateFactory& stateFactory)
		: mTransitionType(transitionType)
		, mpStateFactory(&stateFactory)
	{
		HSM_ASSERT(transitionType != Transition::No);
	}

	Transition::Type GetTransitionType() const
	{
		return mTransitionType;
	}

	StateTypeId GetTargetStateType() const
	{
		HSM_ASSERT(mpStateFactory);
		return mpStateFactory->GetStateType();
	}

	State* CreateTargetState(StateMachine* pOwnerStateMachine) const
	{
		HSM_ASSERT(mpStateFactory);
		return mpStateFactory->CreateState(pOwnerStateMachine);
	}

private:
	Transition::Type mTransitionType;
	StateFactory* mpStateFactory; // Bald pointer because StateFactory instances are always statically allocated
};


// Transition generators - used to return from EvaluateTransition()

template <typename TargetState>
inline Transition SiblingTransition()
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>());
}

template <typename TargetState>
inline Transition InnerTransition()
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>());
}

template <typename TargetState>
inline Transition InnerEntryTransition()
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>());
}

inline Transition NoTransition()
{
	return Transition();
}

#endif // HSM_TRANSITION_H
