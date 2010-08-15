#ifndef GS_RTTI
#define GS_RTTI

#include "Config.h"
#include "Generic.h"

//@NOTE: RTTI must be enabled when compiling
#include <typeinfo>

// Type returned by GetTypeId, can be stored and compared
typedef const std::type_info& TypeId;

// Returns a TypeId for a given object or class
#define GetTypeId typeid

namespace RttiInternal
{
	template <typename T>
	struct GetObjectTypeName
	{
		static const char* Execute(const T& object)
		{
			return GetTypeId(object).name();
		}
	};

	// Specialize for pointers so we can dereference them
	template <typename T>
	struct GetObjectTypeName<T*>
	{
		static const char* Execute(const T* pObject)
		{
			// Recursive call to support getting the final dereferenced object from any
			// chain of pointers (i.e. pointer to pointer to object)
			return GetObjectTypeName<T>::Execute(*pObject);
		}
	};
}

// Returns a unique name for the input object type. If the object type is a pointer, or a chain of pointers, will
// return the name of the final dereferenced object.
template <typename T>
const char* GetObjectTypeName(const T& object) { return RttiInternal::GetObjectTypeName<T>::Execute(object); }

// Returns name of type T
template <typename T>
const char* GetTypeName() { return GetTypeId(T).name(); }

// Attempts to cast pObj of type U to type T, returning a pointer of type T if pObj is of type T, or NULL otherwise.
template <typename T, typename U>
inline T DynamicCast(U pObj)
{
	return dynamic_cast<T>(pObj);
}

// Same as DynamicCast<> but asserts if the cast fails
template <typename T, typename U>
inline T CheckedDynamicCast(U pObj)
{
#if CONFIG_DEBUG
	T pRet = DynamicCast<T>(pObj);
	ASSERT_FORMATTED(pRet != 0, ("CheckedDynamicCast from type %s to %s failed! Actual object type is %s", GetTypeName<U>(), GetTypeName<T>(), GetObjectTypeName(pObj)));
	return pRet;
#else
	return DynamicCast<T>(pObj);
#endif
}

#endif // GS_RTTI
