#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2|resit)

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "vec4.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator [] allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m[1,2];
 *    m[0,3] = 3.f;
 *
 * (Multi-dimensionsal subscripts in operator[] is a C++23 feature!)
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator[] (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator[] (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr
Mat44f operator*( Mat44f const& aLeft, Mat44f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aLeft;   // Avoid warnings about unused arguments until the function
	// (void)aRight;  // is properly implemented.
	// return kIdentity44f;

	 Mat44f result{};

    for (std::size_t i = 0; i < 4; ++i)
    {
        for (std::size_t j = 0; j < 4; ++j)
        {
            float sum = 0.f;
            for (std::size_t k = 0; k < 4; ++k)
            {
                sum += aLeft[i,k] * aRight[k,j];
            }
            result[i,j] = sum;
        }
    }

    return result;
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aLeft;   // Avoid warnings about unused arguments until the function
	// (void)aRight;  // is properly implemented.
	// return { 0.f, 0.f, 0.f, 0.f };

	Vec4f r{};

    r[0] = aLeft[0,0]*aRight[0] + aLeft[0,1]*aRight[1] + aLeft[0,2]*aRight[2] + aLeft[0,3]*aRight[3];
    r[1] = aLeft[1,0]*aRight[0] + aLeft[1,1]*aRight[1] + aLeft[1,2]*aRight[2] + aLeft[1,3]*aRight[3];
    r[2] = aLeft[2,0]*aRight[0] + aLeft[2,1]*aRight[1] + aLeft[2,2]*aRight[2] + aLeft[2,3]*aRight[3];
    r[3] = aLeft[3,0]*aRight[0] + aLeft[3,1]*aRight[1] + aLeft[3,2]*aRight[2] + aLeft[3,3]*aRight[3];

    return r;
}


// Functions:

Mat44f invert( Mat44f const& aM ) noexcept;

inline
Mat44f transpose( Mat44f const& aM ) noexcept
{
	Mat44f ret;
	for( std::size_t i = 0; i < 4; ++i )
	{
		for( std::size_t j = 0; j < 4; ++j )
			ret[j,i] = aM[i,j];
	}
	return ret;
}

inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aAngle; // Avoid warnings about unused arguments until the function
	//               // is properly implemented.
	// return kIdentity44f;

	float ca = std::cos(aAngle);
    float sa = std::sin(aAngle);

    Mat44f rx = kIdentity44f;

    rx[1,1] =  ca; 
	rx[1,2] = -sa;
    rx[2,1] =  sa; 
	rx[2,2] =  ca;

    return rx;

}


inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aAngle; // Avoid warnings about unused arguments until the function
	//               // is properly implemented.
	// return kIdentity44f;

	float ca = std::cos(aAngle);
    float sa = std::sin(aAngle);

    Mat44f ry = kIdentity44f;

    ry[0,0] =  ca;   
	ry[0,2] =  sa;
    ry[2,0] = -sa;  
	ry[2,2] =  ca;

    return ry;
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aAngle; // Avoid warnings about unused arguments until the function
	//               // is properly implemented.
	// return kIdentity44f;

	float ca = std::cos(aAngle);
    float sa = std::sin(aAngle);

    Mat44f rz = kIdentity44f;

    rz[0,0] =  ca;  
	rz[0,1] = -sa;
    rz[1,0] =  sa;  
	rz[1,1] =  ca;

    return rz;
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aTranslation; // Avoid warnings about unused arguments until the function
	//                     // is properly implemented.
	// return kIdentity44f;

	Mat44f t = kIdentity44f;

    t[0,3] = aTranslation[0];
    t[1,3] = aTranslation[1];
    t[2,3] = aTranslation[2];
    return t;
}
inline
Mat44f make_scaling( float aSX, float aSY, float aSZ ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aSX;  // Avoid warnings about unused arguments until the function
	// (void)aSY;  // is properly implemented.
	// (void)aSZ;
	// return kIdentity44f;

	Mat44f im = kIdentity44f;

    im[0,0] = aSX;
    im[1,1] = aSY;
    im[2,2] = aSZ;

    return im;
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aFovInRadians; // Avoid warnings about unused arguments until the function
	// (void)aAspect;       // is properly implemented.
	// (void)aNear;
	// (void)aFar;
	// return kIdentity44f;

	Mat44f p = kIdentity44f;
	float s = 1.f / std::tan( aFovInRadians * 0.5f );

    p[0,0] = s / aAspect;
    p[1,1] = s;

    p[2,2] = -((aFar + aNear) / (aFar - aNear));
    p[2,3] = (-2.f * aFar * aNear) / (aFar - aNear);
	p[3,2] = -1.f;
	p[3,3] = 0.f;

    return p;
}

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
