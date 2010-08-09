#ifndef GS_RTTI
#define GS_RTTI

#include "Config.h"

//@NOTE: RTTI must be enabled when compiling
#include <typeinfo>

typedef const std::type_info& TypeId;

#define GetTypeId typeid

template <typename T, typename U>
inline T DynamicCast(U pObj)
{
	return dynamic_cast<T>(pObj);
}

template <typename T, typename U>
inline T CheckedDynamicCast(U pObj)
{
#if CONFIG_DEBUG
	T pRet = DynamicCast<T>(pObj);
	ASSERT_MSG(pRet != 0, "CheckedDynamicCast failed!");
	return pRet;
#else
	return DynamicCast<T>(pObj);
#endif
}

#endif // GS_RTTI
