#ifndef MAT33_HPP_61F3107B_CBE4_48DE_9F39_EA959B4BF694
#define MAT33_HPP_61F3107B_CBE4_48DE_9F39_EA959B4BF694
// SOLUTION_TAGS: gl-(ex-[^1234]|cw-2|resit)

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "mat44.hpp"

/** Mat33f: 3x3 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator [] allows access to individual elements. Example:
 *    Mat33f m = ...;
 *    float m12 = m[1,2];
 *    m[0,2] = 3.f;
 *
 * (Multi-dimensionsal subscripts in operator[] is a C++23 feature!)
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  ⎞
 *   ⎜ 1,0  1,1  1,2  ⎟
 *   ⎝ 2,0  2,1  2,2  ⎠
 */
struct Mat33f
{
	float v[9];

	constexpr
	float& operator[] (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 3 && aJ < 3 );
		return v[aI*3 + aJ];
	}
	constexpr
	float const& operator[] (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 3 && aJ < 3 );
		return v[aI*3 + aJ];
	}
};

// Identity matrix
constexpr Mat33f kIdentity33f = { {
	1.f, 0.f, 0.f,
	0.f, 1.f, 0.f,
	0.f, 0.f, 1.f
} };

// Common operators for Mat33f.

constexpr
Vec3f operator*( Mat33f const& aLeft, Vec3f const& aRight ) noexcept
{
	Vec3f ret{};
	for( std::size_t j = 0; j < 3; ++j )
	{
		for( std::size_t i = 0; i < 3; ++i )
		{
			ret[j] += aLeft[j,i] * aRight[i];
		}
	}
	return ret;
}

// Functions:

inline
Mat33f mat44_to_mat33( Mat44f const& aM )
{
	Mat33f ret;
	for( std::size_t i = 0; i < 3; ++i )
	{
		for( std::size_t j = 0; j < 3; ++j )
			ret[i,j] = aM[i,j];
	}
	return ret;
}

#endif // MAT33_HPP_61F3107B_CBE4_48DE_9F39_EA959B4BF694
