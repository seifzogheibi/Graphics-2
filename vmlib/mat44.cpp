#include "mat44.hpp"
// SOLUTION_TAGS: gl-(ex-[^1234]|cw-2|resit)

Mat44f invert( Mat44f const& aM ) noexcept
{
	// We could implement this with any number of methods, including Gaussian
	// Elimination or similar. However, straight line solutions exist for small
	// matrices, including 4x4 ones.
	//
	// This particular one is from:
	// http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm
	Mat44f ret;
	ret[0,0] = aM[1,2]*aM[2,3]*aM[3,1] - aM[1,3]*aM[2,2]*aM[3,1] 
		+ aM[1,3]*aM[2,1]*aM[3,2] - aM[1,1]*aM[2,3]*aM[3,2] 
		- aM[1,2]*aM[2,1]*aM[3,3] + aM[1,1]*aM[2,2]*aM[3,3];
	ret[0,1] = aM[0,3]*aM[2,2]*aM[3,1] - aM[0,2]*aM[2,3]*aM[3,1] 
		- aM[0,3]*aM[2,1]*aM[3,2] + aM[0,1]*aM[2,3]*aM[3,2] 
		+ aM[0,2]*aM[2,1]*aM[3,3] - aM[0,1]*aM[2,2]*aM[3,3];
	ret[0,2] = aM[0,2]*aM[1,3]*aM[3,1] - aM[0,3]*aM[1,2]*aM[3,1] 
		+ aM[0,3]*aM[1,1]*aM[3,2] - aM[0,1]*aM[1,3]*aM[3,2] 
		- aM[0,2]*aM[1,1]*aM[3,3] + aM[0,1]*aM[1,2]*aM[3,3];
	ret[0,3] = aM[0,3]*aM[1,2]*aM[2,1] - aM[0,2]*aM[1,3]*aM[2,1] 
		- aM[0,3]*aM[1,1]*aM[2,2] + aM[0,1]*aM[1,3]*aM[2,2] 
		+ aM[0,2]*aM[1,1]*aM[2,3] - aM[0,1]*aM[1,2]*aM[2,3];
	ret[1,0] = aM[1,3]*aM[2,2]*aM[3,0] - aM[1,2]*aM[2,3]*aM[3,0] 
		- aM[1,3]*aM[2,0]*aM[3,2] + aM[1,0]*aM[2,3]*aM[3,2] 
		+ aM[1,2]*aM[2,0]*aM[3,3] - aM[1,0]*aM[2,2]*aM[3,3];
	ret[1,1] = aM[0,2]*aM[2,3]*aM[3,0] - aM[0,3]*aM[2,2]*aM[3,0] 
		+ aM[0,3]*aM[2,0]*aM[3,2] - aM[0,0]*aM[2,3]*aM[3,2] 
		- aM[0,2]*aM[2,0]*aM[3,3] + aM[0,0]*aM[2,2]*aM[3,3];
	ret[1,2] = aM[0,3]*aM[1,2]*aM[3,0] - aM[0,2]*aM[1,3]*aM[3,0] 
		- aM[0,3]*aM[1,0]*aM[3,2] + aM[0,0]*aM[1,3]*aM[3,2] 
		+ aM[0,2]*aM[1,0]*aM[3,3] - aM[0,0]*aM[1,2]*aM[3,3];
	ret[1,3] = aM[0,2]*aM[1,3]*aM[2,0] - aM[0,3]*aM[1,2]*aM[2,0] 
		+ aM[0,3]*aM[1,0]*aM[2,2] - aM[0,0]*aM[1,3]*aM[2,2] 
		- aM[0,2]*aM[1,0]*aM[2,3] + aM[0,0]*aM[1,2]*aM[2,3];
	ret[2,0] = aM[1,1]*aM[2,3]*aM[3,0] - aM[1,3]*aM[2,1]*aM[3,0] 
		+ aM[1,3]*aM[2,0]*aM[3,1] - aM[1,0]*aM[2,3]*aM[3,1] 
		- aM[1,1]*aM[2,0]*aM[3,3] + aM[1,0]*aM[2,1]*aM[3,3];
	ret[2,1] = aM[0,3]*aM[2,1]*aM[3,0] - aM[0,1]*aM[2,3]*aM[3,0] 
		- aM[0,3]*aM[2,0]*aM[3,1] + aM[0,0]*aM[2,3]*aM[3,1] 
		+ aM[0,1]*aM[2,0]*aM[3,3] - aM[0,0]*aM[2,1]*aM[3,3];
	ret[2,2] = aM[0,1]*aM[1,3]*aM[3,0] - aM[0,3]*aM[1,1]*aM[3,0] 
		+ aM[0,3]*aM[1,0]*aM[3,1] - aM[0,0]*aM[1,3]*aM[3,1] 
		- aM[0,1]*aM[1,0]*aM[3,3] + aM[0,0]*aM[1,1]*aM[3,3];
	ret[2,3] = aM[0,3]*aM[1,1]*aM[2,0] - aM[0,1]*aM[1,3]*aM[2,0] 
		- aM[0,3]*aM[1,0]*aM[2,1] + aM[0,0]*aM[1,3]*aM[2,1] 
		+ aM[0,1]*aM[1,0]*aM[2,3] - aM[0,0]*aM[1,1]*aM[2,3];
	ret[3,0] = aM[1,2]*aM[2,1]*aM[3,0] - aM[1,1]*aM[2,2]*aM[3,0] 
		- aM[1,2]*aM[2,0]*aM[3,1] + aM[1,0]*aM[2,2]*aM[3,1] 
		+ aM[1,1]*aM[2,0]*aM[3,2] - aM[1,0]*aM[2,1]*aM[3,2];
	ret[3,1] = aM[0,1]*aM[2,2]*aM[3,0] - aM[0,2]*aM[2,1]*aM[3,0] 
		+ aM[0,2]*aM[2,0]*aM[3,1] - aM[0,0]*aM[2,2]*aM[3,1] 
		- aM[0,1]*aM[2,0]*aM[3,2] + aM[0,0]*aM[2,1]*aM[3,2];
	ret[3,2] = aM[0,2]*aM[1,1]*aM[3,0] - aM[0,1]*aM[1,2]*aM[3,0] 
		- aM[0,2]*aM[1,0]*aM[3,1] + aM[0,0]*aM[1,2]*aM[3,1] 
		+ aM[0,1]*aM[1,0]*aM[3,2] - aM[0,0]*aM[1,1]*aM[3,2];
	ret[3,3] = aM[0,1]*aM[1,2]*aM[2,0] - aM[0,2]*aM[1,1]*aM[2,0] 
		+ aM[0,2]*aM[1,0]*aM[2,1] - aM[0,0]*aM[1,2]*aM[2,1] 
		- aM[0,1]*aM[1,0]*aM[2,2] + aM[0,0]*aM[1,1]*aM[2,2];

	float const d = aM[0,0] * ret[0,0] + aM[0,1] * ret[1,0] 
		+ aM[0,2] * ret[2,0] + aM[0,3] * ret[3,0];

	for( auto& v : ret.v )
		v /= d;

	return ret;
}

