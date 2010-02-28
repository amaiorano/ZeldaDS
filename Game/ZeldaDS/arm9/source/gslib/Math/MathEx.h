#ifndef MATH_EX_H
#define MATH_EX_H

// Math Extensions

#include <cmath>
#include <cstdlib>

namespace MathEx
{
	// Simply clamps a value to range [min, max]
	template <typename T, typename U, typename V>
	T Clamp(const T& val, U min, V max)
	{
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}

	// Allows scaling of different types (i.e. scaling an int by a float value)
	template <typename T, typename U>
	inline T Scale(T val, U scaleFactor)
	{
		return static_cast<T>((static_cast<U>(val) * scaleFactor));
	}

	// Performs lhs % rhs for Real numbers (non-integer)
	template <typename T>
	inline T RealModulo(T lhs, T rhs) { return ::fmod(lhs, rhs); }

	// Returns value to some exponent
	template <typename T>
	inline T Pow(T val, T exp) { return ::pow(val, exp); }

	// Returns value squared
	template <typename T>
	inline T Pow2(T val) { return val*val; }

	// Returns square root of value
	template <typename T>
	inline T SquareRoot(T val) { return ::sqrt(val); }

	// Returns smallest integer that is >= to val
	template <typename T>
	inline T Ceil(T val) { return ::ceil(val); }

	// Returns largest integer that is <= to val
	template <typename T>
	inline T Floor(T val) { return ::floor(val); }

	// Returns absolute value
	template <typename T>
	inline T Abs(T val) { return abs(val); }

	// Returns true if input value is a power of two (2, 4, 8, 16, 32, etc.)
	template <typename T>
	inline bool IsPowerOfTwo(T val)
	{
		int v = static_cast<int>(val);
		return !(v & (v-1));
	}

	// Returns a random value in the range [min, max]
	template <typename T>
	inline T Rand(T min, T max)
	{
		return rand() % (max-min+1) + min;
	}

	inline float Rand(float min, float max)
	{
		float ret = rand()/(((float)RAND_MAX + 1) / (max-min));
		return ret + min;
	}

	// Returns a random value in the range [0, max]
	template <typename T>
	inline T Rand(T max)
	{
		return Rand<T>(0, max);
	}

	inline float Rand(float max)
	{
		return Rand(0.0f, max);
	}
}

#endif // MATH_EX_H
