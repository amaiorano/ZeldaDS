#ifndef GS_RTTI
#define GS_RTTI

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
#ifdef NDEBUG
	return DynamicCast<T>(pObj);
#else
	T pRet = DynamicCast<T>(pObj);
	ASSERT_MSG(pRet != 0, "Checked cast failed!");
	return pRet;
#endif
}


#endif // GS_RTTI
