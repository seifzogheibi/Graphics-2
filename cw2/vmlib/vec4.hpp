#ifndef VEC4_HPP_7524F057_7AA7_4C99_AA52_DB0B5A3F8CAA
#define VEC4_HPP_7524F057_7AA7_4C99_AA52_DB0B5A3F8CAA
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2|resit)

#include <cmath>
#include <cassert>
#include <cstdlib>

struct Vec4f
{
	float x, y, z, w;

	constexpr 
	float& operator[] (std::size_t aI) noexcept
	{
		assert( aI < 4 );
		return aI[&x]; // This is a bit sketchy.
	}
	constexpr 
	float operator[] (std::size_t aI) const noexcept
	{
		assert( aI < 4 );
		return aI[&x]; // This is a bit sketchy.
	}
};


constexpr
Vec4f operator+( Vec4f aVec ) noexcept
{
	return aVec;
}
constexpr
Vec4f operator-( Vec4f aVec ) noexcept
{
	return { -aVec.x, -aVec.y, -aVec.z, -aVec.w };
}

constexpr
Vec4f operator+( Vec4f aLeft, Vec4f aRight ) noexcept
{
	return Vec4f{
		aLeft.x + aRight.x,
		aLeft.y + aRight.y,
		aLeft.z + aRight.z,
		aLeft.w + aRight.w
	};
}
constexpr
Vec4f operator-( Vec4f aLeft, Vec4f aRight ) noexcept
{
	return Vec4f{
		aLeft.x - aRight.x,
		aLeft.y - aRight.y,
		aLeft.z - aRight.z,
		aLeft.w - aRight.w
	};
}


constexpr
Vec4f operator*( float aScalar, Vec4f aVec ) noexcept
{
	return Vec4f{ 
		aScalar * aVec.x, 
		aScalar * aVec.y, 
		aScalar * aVec.z, 
		aScalar * aVec.w 
	};
}
constexpr
Vec4f operator*( Vec4f aVec, float aScalar ) noexcept
{
	return aScalar * aVec;
}

constexpr
Vec4f operator/( Vec4f aVec, float aScalar ) noexcept
{
	return Vec4f{ 
		aVec.x / aScalar,
		aVec.y / aScalar,
		aVec.z / aScalar,
		aVec.w / aScalar
	};
}


constexpr
Vec4f& operator+=( Vec4f& aLeft, Vec4f aRight ) noexcept
{
	aLeft.x += aRight.x;
	aLeft.y += aRight.y;
	aLeft.z += aRight.z;
	aLeft.w += aRight.w;
	return aLeft;
}
constexpr
Vec4f& operator-=( Vec4f& aLeft, Vec4f aRight ) noexcept
{
	aLeft.x -= aRight.x;
	aLeft.y -= aRight.y;
	aLeft.z -= aRight.z;
	aLeft.w -= aRight.w;
	return aLeft;
}

constexpr
Vec4f& operator*=( Vec4f& aLeft, float aRight ) noexcept
{
	aLeft.x *= aRight;
	aLeft.y *= aRight;
	aLeft.z *= aRight;
	aLeft.w *= aRight;
	return aLeft;
}
constexpr
Vec4f& operator/=( Vec4f& aLeft, float aRight ) noexcept
{
	aLeft.x /= aRight;
	aLeft.y /= aRight;
	aLeft.z /= aRight;
	aLeft.w /= aRight;
	return aLeft;
}


// A few common functions:

constexpr
float dot( Vec4f aLeft, Vec4f aRight ) noexcept
{
	return aLeft.x * aRight.x 
		+ aLeft.y * aRight.y
		+ aLeft.z * aRight.z
		+ aLeft.w * aRight.w
	;
}

inline
float length( Vec4f aVec ) noexcept
{
	// The standard function std::sqrt() is not marked as constexpr. length()
	// calls std::sqrt() unconditionally, so length() cannot be marked
	// constexpr itself.
	return std::sqrt( dot( aVec, aVec ) );
}


#endif // VEC4_HPP_7524F057_7AA7_4C99_AA52_DB0B5A3F8CAA
