#ifndef CORE_TYPES_H
#define CORE_TYPES_H

// Simple, we just use the types from libnds
#include <nds/ndstypes.h>

template <typename T> class Vector2;
typedef Vector2<int16> Vector2I;
typedef Vector2<float> Vector2F;

#endif // CORE_TYPES_H
