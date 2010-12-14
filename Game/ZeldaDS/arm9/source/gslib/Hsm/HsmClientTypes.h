#ifndef HSM_CLIENT_TYPES_H
#define HSM_CLIENT_TYPES_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Rtti.h"
#include "gslib/Util/intrusive_ptr.h"
#include <vector>

#if CONFIG_DEBUG
#define HSM_DEBUG
#endif

typedef uint16 HsmTimeType; // Elapsed frames
#define HSM_STD_VECTOR std::vector
#define HSM_INTRUSIVE_PTR intrusive_ptr
#define HSM_INTRUSVE_PTR_CLIENT intrusive_ptr_client
#define HSM_ASSERT ASSERT
#define HSM_PRINTF printf

// StateTypeId: requirements are that StateTypeId be unique per State type, and that they can be compared using operator==
typedef TypeId StateTypeId;

// GetStateTypeId(obj/class) must return a StateTypeId
#define GetStateTypeId(state) GetTypeId(state)

// GetStateDebugName returns the name of a state for debug output only (not necessary)
#define GetStateDebugName(state) GetObjectTypeName(state)

#endif // HSM_CLIENT_TYPES_H
