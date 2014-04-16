#ifndef HSM_UTIL_H
#define HSM_UTIL_H

#include "hsm/StateMachine.h"

struct SharedStateData
{
	virtual ~SharedStateData() {}
};

template <typename OwnerType, typename SharedStateDataType, typename StateBaseType = hsm::State>
struct StateWithOwnerAndData : hsm::StateWithOwner<OwnerType, StateBaseType>
{
	using hsm::StateWithOwner<OwnerType, StateBaseType>::Owner;

	const SharedStateDataType& Data() const
	{
		return *static_cast<SharedStateDataType*>(Owner().GetSharedStateData());
	}

	SharedStateDataType& Data()
	{
		return *static_cast<SharedStateDataType*>(Owner().GetSharedStateData());
	}
};

class IHsmOwner
{
public:
	IHsmOwner() : mpSharedStateData(NULL) {}
	~IHsmOwner() { delete mpSharedStateData; }

	SharedStateData* GetSharedStateData() { return mpSharedStateData; }
	const SharedStateData* GetSharedStateData() const { return mpSharedStateData; }

protected:
	SharedStateData* mpSharedStateData;
};


#endif // HSM_UTIL_H
