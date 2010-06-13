#ifndef VECTOR2_H
#define VECTOR2_H

#include "MathEx.h"

// Can be used by multiple types
enum InitZeroTag { InitZero };

template <typename T>
class Vector2
{
public:
	T x;
	T y;

	Vector2() { }
	Vector2(T inX, T inY) : x(inX), y(inY) { }
	Vector2(InitZeroTag) : x(0), y(0) { }

	// Conversion constructor
	template <typename U>
	Vector2(const Vector2<U>& rhs)
	{
		Reset(rhs);
	}

	template <typename U>
	void Reset(const Vector2<U>& rhs)
	{
		x = static_cast<T>(rhs.x);
		y = static_cast<T>(rhs.y);
	}

	void Reset(T inX, T inY)
	{
		x = inX;
		y = inY;
	}

	void Reset(InitZeroTag)
	{
		x = 0;
		y = 0;
	}

	T Length() const
	{
		return MathEx::SquareRoot(x*x + y*y);
	}

	Vector2& Normalize()
	{
		*this /= Length();
		return *this;
	}

	void ScaleComponents(const Vector2& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
	}

	void operator+=(const Vector2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
	}

	void operator-=(const Vector2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
	}

	template <typename U>
	void operator*=(const U& s)
	{
		x *= s;
		y *= s;
	}

	template <typename U>
	void operator/=(const U& s)
	{
		x /= s;
		y /= s;
	}

	Vector2 operator-() const // Unary minus operator
	{
		return Vector2(-x, -y);
	}
};

template <typename T>
inline Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	Vector2<T> result = lhs;
	result += rhs;
	return result;
}

template <typename T>
inline Vector2<T> operator-(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	Vector2<T> result = lhs;
	result -= rhs;
	return result;
}

template <typename T, typename U>
inline Vector2<T> operator*(const Vector2<T>& lhs, const U& s)
{
	Vector2<T> result = lhs;
	result *= s;
	return result;
}

template <typename T, typename U>
inline Vector2<T> operator*(const U& s, const Vector2<T>& rhs)
{
	Vector2<T> result = rhs;
	result *= s;
	return result;
}

template <typename T, typename U>
inline Vector2<T> operator/(const Vector2<T>& lhs, const U& s)
{
	Vector2<T> result = lhs;
	result /= s;
	return result;
}

template <typename T>
inline Vector2<T> Normalized(const Vector2<T>& v)
{
	Vector2<T> result = v;
	return result.Normalize();
}

template <typename T, typename U>
inline Vector2<T> ScaleComponents(const Vector2<T>& lhs, const Vector2<U>& rhs)
{
	Vector2<T> result = lhs;
	result.ScaleComponents(rhs);
	return result;
}

template <typename T, typename U>
inline T Dot(const Vector2<T>& lhs, const Vector2<U>& rhs)
{
	return (lhs.x * rhs.x + lhs.y * rhs.y);
}

#endif // VECTOR2_H
