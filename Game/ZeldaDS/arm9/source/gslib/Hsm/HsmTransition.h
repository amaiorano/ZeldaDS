#ifndef HSM_TRANSITION_H
#define HSM_TRANSITION_H

#include "HsmState.h"

///////////////////////////////////////////////////////////////////////////////
// StateFactory
///////////////////////////////////////////////////////////////////////////////

struct StateFactory;

// Returns the one StateFactory instance for the input state
template <typename TargetState>
const StateFactory& GetStateFactory();

// StateFactory is responsible for creating State instances
struct StateFactory
{
	virtual StateTypeId GetStateType() const = 0;
	virtual State* AllocState() const = 0;
	virtual void InvokeStateOnEnter(State* pState, const StateArgs* pStateArgs) const = 0;
};

namespace HsmInternal
{
	template <bool condition, typename TrueType, typename FalseType>
	struct Select
	{
		typedef TrueType Type;
	};

	template <typename TrueType, typename FalseType>
	struct Select<false, TrueType, FalseType>
	{
		typedef FalseType Type;
	};
}

template <typename TargetState>
struct ConcreteStateFactory : StateFactory
{
	virtual StateTypeId GetStateType() const
	{
		return GetStateTypeId(TargetState);
	}

	virtual State* AllocState() const
	{
		return new TargetState();
	}

	virtual void InvokeStateOnEnter(State* pState, const StateArgs* pStateArgs) const
	{
		// We select which functor to call at compile-time. We do this so that only states that expect StateArgs are required
		// to have an OnEnter(const Args& args) where Args is the derived struct from StateArgs defined in TargetState.
		const bool expectsStateArgs = sizeof(typename TargetState::Args) > sizeof(State::Args);

		typedef typename HsmInternal::Select<expectsStateArgs, InvokeStateOnEnterWithStateArgsFunctor, InvokeStateOnEnterFunctor>::Type Functor;
		Functor::Execute(pState, pStateArgs);
	}

	struct InvokeStateOnEnterFunctor
	{
		static void Execute(State* pState, const StateArgs* pStateArgs)
		{
			HSM_ASSERT(pStateArgs == 0 && "Target state does not expect args, but args were passed in via the transition");

			//@NOTE: Compiler will fail if TargetState defines OnEnter(const Args&)
			static_cast<TargetState*>(pState)->OnEnter();
		}
	};

	struct InvokeStateOnEnterWithStateArgsFunctor
	{
		static void Execute(State* pState, const StateArgs* pStateArgs)
		{
			HSM_ASSERT(pStateArgs && "Target state expects args, but none were passed in via the transition");

			//@NOTE: Compiler will fail if TargetState does not define OnEnter(const Args&)
			static_cast<TargetState*>(pState)->OnEnter(static_cast<const typename TargetState::Args&>(*pStateArgs));
		}
	};

private:
	friend const StateFactory& GetStateFactory<TargetState>();
	ConcreteStateFactory() { }
};

template <typename TargetState>
const StateFactory& GetStateFactory()
{
	static ConcreteStateFactory<TargetState> instance;
	return instance;
}


///////////////////////////////////////////////////////////////////////////////
// Transition
///////////////////////////////////////////////////////////////////////////////

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

	// With StateFactory
	Transition(Transition::Type transitionType, const StateFactory& stateFactory)
		: mTransitionType(transitionType)
		, mpStateFactory(&stateFactory)
	{
		HSM_ASSERT(transitionType != Transition::No);
	}

	// With StateFactory and StateArgs
	template <typename StateArgsType>
	Transition(Transition::Type transitionType, const StateFactory& stateFactory, const StateArgsType& stateArgs)
		: mTransitionType(transitionType)
		, mpStateFactory(&stateFactory)
	{
		HSM_ASSERT(transitionType != Transition::No);
		mpStateArgs.reset(new StateArgsType(stateArgs)); // Copy-construct new instance for intrusive_ptr to hold
	}

	Transition::Type GetTransitionType() const
	{
		return mTransitionType;
	}

	StateTypeId GetTargetStateType() const
	{
		return mpStateFactory->GetStateType();
	}

	const StateFactory& GetStateFactory() const
	{
		return *mpStateFactory;
	}

	const StateArgs* GetStateArgs() const // Do not cache returned pointer
	{
		return mpStateArgs.get();
	}

private:
	Transition::Type mTransitionType;
	const StateFactory* mpStateFactory; // Bald pointer is safe because StateFactory instances are always statically allocated
	HSM_INTRUSIVE_PTR<const StateArgs> mpStateArgs; // Reference counted pointer so we can safely copy Transitions without leaking
};


// Transition generators - used to return from EvaluateTransition()

template <typename TargetState>
inline Transition SiblingTransition()
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>());
}

template <typename TargetState, typename StateArgsType>
inline Transition SiblingTransition(const StateArgsType& stateArgs)
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>(), stateArgs);
}

template <typename TargetState>
inline Transition InnerTransition()
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>());
}

template <typename TargetState, typename StateArgsType>
inline Transition InnerTransition(const StateArgsType& stateArgs)
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>(), stateArgs);
}

template <typename TargetState>
inline Transition InnerEntryTransition()
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>());
}

template <typename TargetState, typename StateArgsType>
inline Transition InnerEntryTransition(const StateArgsType& stateArgs)
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>(), stateArgs);
}

inline Transition NoTransition()
{
	return Transition();
}

#endif // HSM_TRANSITION_H
