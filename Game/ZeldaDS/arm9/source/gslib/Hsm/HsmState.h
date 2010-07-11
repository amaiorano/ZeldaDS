#ifndef HSM_STATE_H
#define HSM_STATE_H

#include "HsmClientTypes.h"
#include <vector>

struct Transition;
struct State;
class StateMachine;

// Base class for visitor functors
struct StateVisitor
{
	// Return true to keep visiting states, false to stop
	virtual bool OnVisit(State* pState, void* pUserData = NULL) = 0;
};

// Base class for shared data for all states
struct SharedStateData
{
	virtual ~SharedStateData() {}
};


///////////////////////////////////////////////////////////////////////////////
// State Attributes
///////////////////////////////////////////////////////////////////////////////

template <typename  T>
struct ConcreteAttributeResetter;

template <typename T>
struct Attribute
{
	// Constructor - allows client to initialize attribute; afterward,
	// can only assign via State::SetAttribute().
	// Note that T must be default constructable.
	explicit Attribute(const T& initValue = T()) { m_value = initValue; }

	// Conversion operator to const T& (but not T&)
	operator const T&() const { return m_value; }

	// Provided for clients who don't want to cast
	const T& Value() const { return m_value; }

private:
	// Disable copy
	Attribute(const Attribute& rhs);
	Attribute& operator=(Attribute& rhs);

	friend struct ConcreteAttributeResetter<T>;
	T m_value;
};

struct AttributeResetter
{
	virtual ~AttributeResetter() { }
};

template <typename  T>
struct ConcreteAttributeResetter : public AttributeResetter
{
	ConcreteAttributeResetter(Attribute<T>& attrib, const T& newValue)
	{
		m_pAttrib = &attrib;
		m_origValue = attrib.m_value;
		attrib.m_value = newValue;
	}

	virtual ~ConcreteAttributeResetter()
	{
		m_pAttrib->m_value = m_origValue;
	}

	Attribute<T>* m_pAttrib;
	T m_origValue;
};

namespace HsmInternal
{
	inline void InitState(State* pState, StateMachine* pOwnerStateMachine);
}

///////////////////////////////////////////////////////////////////////////////
// State
///////////////////////////////////////////////////////////////////////////////

struct State
{
	State() 
		: m_pOwnerStateMachine(NULL)
		, m_attributeResetters(0)
	{
	}
	
	virtual ~State()
	{
		ResetAttributes();
	}

	// Accessors
	StateTypeId GetStateType() const { return GetStateTypeId(*this); }	
	StateMachine& GetStateMachine() { HSM_ASSERT(m_pOwnerStateMachine); return *m_pOwnerStateMachine; }

	// Convenience functions
	template <typename ChildState>
	inline ChildState* FindState();

	template <typename ChildState>
	ChildState* GetState()
	{
		ChildState* pState = FindState<ChildState>();
		HSM_ASSERT(pState);
		return pState;
	}

	template <typename ChildState>
	inline bool IsInState();
	
	// Client can implement these
	// Note: GetStateMachine() is valid in OnEnter(), but not in a state constructor
	void OnEnter() { } // Doesn't need to be virtual - called directly on child state
	virtual void OnExit() { }
	virtual Transition& EvaluateTransitions(HsmTimeType deltaTime); // Called from outer to inner until settled on a state (no more transitions returned)
	virtual void PerformStateActions(HsmTimeType deltaTime) { }; // Called from outer to inner once settled on a state (after StateMachine::EvaluateStateTransitions())

	// Should be called from OnEnter() to override an attribute while in current state
	template <typename T, typename U>
	void SetAttribute(Attribute<T>& attrib, const U& newValue)
	{
		// Optimization: no need to push an attribute if it already contains the new value
		if (attrib.Value() == newValue)
			return;

		#ifdef HSM_DEBUG
		// Don't allow setting an attribute twice from a state (probably a misuse of attributes)
		AttributeResetterList::iterator iter = m_attributeResetters.begin();
		const AttributeResetterList::iterator& iterEnd = m_attributeResetters.end();
		for ( ; iter != iterEnd; ++iter)
		{
			HSM_ASSERT(&attrib != static_cast<ConcreteAttributeResetter<T>*>(*iter)->m_pAttrib);
		}
		#endif

		//@TODO: Get allocator from state machine and replace this 'new'
		m_attributeResetters.push_back( new ConcreteAttributeResetter<T>(attrib, newValue) );
	}

private:
	friend inline void HsmInternal::InitState(State* pState, StateMachine* pOwnerStateMachine);
	StateMachine* m_pOwnerStateMachine;

	void ResetAttributes()
	{
		// Destroy attributes (will reset to old value)
		AttributeResetterList::iterator iter = m_attributeResetters.begin();
		const AttributeResetterList::iterator& iterEnd = m_attributeResetters.end();
		for ( ; iter != iterEnd; ++iter)
		{
			delete *iter;
		}
		m_attributeResetters.clear();
	}

	typedef std::vector<AttributeResetter*> AttributeResetterList;
	AttributeResetterList m_attributeResetters;
};


///////////////////////////////////////////////////////////////////////////////
// ClientStateBase
///////////////////////////////////////////////////////////////////////////////

struct NoOwner { };

// Optional class that clients can use instead of deriving directly from State to allow for
// more convenient semantics within their state code.
template <typename SharedStateDataChild, typename OwnerType = NoOwner>
struct ClientStateBase : public State
{
	OwnerType& Owner()
	{
		HSM_ASSERT(GetStateMachine().GetOwner());
		return *static_cast<OwnerType*>(GetStateMachine().GetOwner());
	}

	SharedStateDataChild& Data()
	{
		return static_cast<SharedStateDataChild&>(GetStateMachine().GetSharedStateData());
	}
};

///////////////////////////////////////////////////////////////////////////////
// State Creation/Destruction
///////////////////////////////////////////////////////////////////////////////

namespace HsmInternal
{
	inline void InitState(State* pState, StateMachine* pOwnerStateMachine)
	{
		pState->m_pOwnerStateMachine = pOwnerStateMachine;
	}
}

template <typename ChildState>
State* CreateState(StateMachine* pOwnerStateMachine)
{
	ChildState* pState = new ChildState();
	HSM_ASSERT(pOwnerStateMachine);
	HsmInternal::InitState(pState, pOwnerStateMachine);
	pState->OnEnter();
	return pState;
}

inline void DestroyState(State* pState)
{
	pState->OnExit();
	delete pState;
}


struct NullState : public State
{
};

#endif // HSM_STATE_H
