#ifndef HSM_CLIENT_TYPES_H
#define HSM_CLIENT_TYPES_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Rtti.h"

#ifndef NDEBUG
#define HSM_DEBUG
#endif

typedef uint16 HsmTimeType; // Elapsed frames
#define HSM_ASSERT ASSERT
#define HSM_PRINTF printf

// StateTypeId: requirements are that StateTypeId be unique per State type, and that they can be compared using operator==
typedef TypeId StateTypeId;

// GetStateTypeId(obj/class) must return a StateTypeId
#define GetStateTypeId GetTypeId

#endif // HSM_CLIENT_TYPES_H
