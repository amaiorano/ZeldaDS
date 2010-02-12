#ifndef HSM_TRANSITION_H
#define HSM_TRANSITION_H

#include "HsmState.h"

// Transition hierarchy
struct Transition
{
	enum Type { Sibling, Inner, InnerEntry, No };

	virtual Transition::Type GetTransitionType() const = 0;
	virtual StateTypeId GetStateType() const = 0;
	virtual State* CreateState(StateMachine* pOwnerStateMachine) const = 0;
};

template <typename ChildState, int transType>
struct ConcreteTransition : public Transition
{	
	virtual Transition::Type GetTransitionType() const	{ return static_cast<Transition::Type>(transType); }
	virtual StateTypeId GetStateType() const			{ return GetStateTypeId(ChildState); }

	virtual State* CreateState(StateMachine* pOwnerStateMachine) const
	{
		return ::CreateState<ChildState>(pOwnerStateMachine);
	}
};

// Transition generators - used to return from EvaluateTransition()

template <typename TargetState>
inline Transition& SiblingTransition()
{
	static ConcreteTransition<TargetState, Transition::Sibling> transition;
	return transition;
}

template <typename TargetState>
inline Transition& InnerTransition()
{
	static ConcreteTransition<TargetState, Transition::Inner> transition;
	return transition;
}

template <typename TargetState>
inline Transition& InnerEntryTransition()
{
	static ConcreteTransition<TargetState, Transition::InnerEntry> transition;
	return transition;
}

inline Transition& NoTransition()
{
	static ConcreteTransition<NullState, Transition::No> transition;
	return transition;
}


#endif // HSM_TRANSITION_H
