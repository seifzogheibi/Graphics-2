#ifndef VEC2_HPP_E2534611_CE12_4452_8ACE_ACBCF5C47CAE
#define VEC2_HPP_E2534611_CE12_4452_8ACE_ACBCF5C47CAE

#include <cmath>

/** Vec2f : 2D vector with floats
 *
 * Purposefully keeping it simple: Vec2f is a POD (Plain Old Data) type. This
 * has some guarantees relating to the layout of the members in memory. Among
 * other things, we can safely memcpy() Vec2fs around.
 *
 * To initialize a Vec2f, you can use braced initialization:
 *   Vec2f a{ 1.f, 2.f, }; // a.x == 1, a.y == 2
 *   Vec2f b = { 1.f, 2.f };
 * Temporary Vec2fs can be constructed similarly:
 *   function_taking_vec2f_argument( Vec2f{ 1.f, 2.f } );
 *   function_taking_vec2f_argument( { 1.f, 2.f } );
 */
struct Vec2f
{
	float x, y;
};


// Define a few common operators for Vec2f
// Note: these are declared "constexpr". Constexpr implies inline, but also
// indicates that the function can be evaluated at compile time. Constexpr
// functions have some additional limitations compared to normal functions.

constexpr
Vec2f operator+( Vec2f aVec ) noexcept
{
	return aVec;
}
constexpr
Vec2f operator-( Vec2f aVec ) noexcept
{
	return { -aVec.x, -aVec.y };
}

constexpr
Vec2f operator+( Vec2f aLeft, Vec2f aRight ) noexcept
{
	return Vec2f{
		aLeft.x + aRight.x,
		aLeft.y + aRight.y
	};
}
constexpr
Vec2f operator-( Vec2f aLeft, Vec2f aRight ) noexcept
{
	return Vec2f{
		aLeft.x - aRight.x,
		aLeft.y - aRight.y
	};
}


constexpr
Vec2f operator*( float aScalar, Vec2f aVec ) noexcept
{
	return Vec2f{ aScalar * aVec.x, aScalar * aVec.y };
}
constexpr
Vec2f operator*( Vec2f aVec, float aScalar ) noexcept
{
	return Vec2f{ aVec.x * aScalar, aVec.y * aScalar };
}

constexpr
Vec2f operator/( Vec2f aVec, float aScalar ) noexcept
{
	return Vec2f{ aVec.x / aScalar, aVec.y / aScalar };
}


constexpr
Vec2f& operator+=( Vec2f& aLeft, Vec2f aRight ) noexcept
{
	aLeft.x += aRight.x;
	aLeft.y += aRight.y;
	return aLeft;
}
constexpr
Vec2f& operator-=( Vec2f& aLeft, Vec2f aRight ) noexcept
{
	aLeft.x -= aRight.x;
	aLeft.y -= aRight.y;
	return aLeft;
}

constexpr
Vec2f& operator*=( Vec2f& aLeft, float aRight ) noexcept
{
	aLeft.x *= aRight;
	aLeft.y *= aRight;
	return aLeft;
}
constexpr
Vec2f& operator/=( Vec2f& aLeft, float aRight ) noexcept
{
	aLeft.x /= aRight;
	aLeft.y /= aRight;
	return aLeft;
}


// A few common functions:

constexpr
float dot( Vec2f aLeft, Vec2f aRight ) noexcept
{
	return aLeft.x * aRight.x + aLeft.y * aRight.y;
}

inline
float length( Vec2f aVec ) noexcept
{
	// The standard function std::sqrt() is not marked as constexpr. length()
	// calls std::sqrt() unconditionally, so length() cannot be marked
	// constexpr itself.
	return std::sqrt( dot( aVec, aVec ) );
}


#endif // VEC2_HPP_E2534611_CE12_4452_8ACE_ACBCF5C47CAE
