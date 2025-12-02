// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.

#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"

// Comparing floating point values isn't straight forward. See
// https://github.com/catchorg/Catch2/blob/devel/docs/comparing-floating-point-numbers.md#top
// for facilities that Catch2 provides.
//
// Note that we still need to decide what kind of errors we want to accept (so,
// absolute, relative or ULP error). The examples take a simple approach of 
// permitting an absolute error of 10^-6. This should be mostly good enough
// for our purposes. (We could likely be stricter; std::sin() and similar are
// likely accurate to 1 ULP for "normal" floats.)


// Note: as before, these checks are not exhaustive. They're meant as examples
// to get you started.

TEST_CASE( "4x4 rotation around Z axis", "[rotation][mat44]" )
{
	static constexpr float kEps_ = 1e-6f;

	using namespace Catch::Matchers;

	// Simple check: rotating zero degrees should yield an idenity matrix
	SECTION( "Identity" )
	{
		auto const identity = make_rotation_z( 0.f );

		REQUIRE_THAT( (identity[0,0]), WithinAbs( 1.f, kEps_ ) );
		REQUIRE_THAT( (identity[0,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[0,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[0,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (identity[1,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[1,1]), WithinAbs( 1.f, kEps_ ) );
		REQUIRE_THAT( (identity[1,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[1,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (identity[2,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[2,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[2,2]), WithinAbs( 1.f, kEps_ ) );
		REQUIRE_THAT( (identity[2,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (identity[3,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[3,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[3,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (identity[3,3]), WithinAbs( 1.f, kEps_ ) );
	}

	// Rotating 90 degrees = pi/2 radians.
	SECTION( "90 degrees" )
	{
		auto const right = make_rotation_z( std::numbers::pi_v<float>/2.f );

		REQUIRE_THAT( (right[0,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[0,1]), WithinAbs( -1.f, kEps_ ) );
		REQUIRE_THAT( (right[0,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[0,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (right[1,0]), WithinAbs( 1.f, kEps_ ) );
		REQUIRE_THAT( (right[1,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[1,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[1,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (right[2,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[2,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[2,2]), WithinAbs( 1.f, kEps_ ) );
		REQUIRE_THAT( (right[2,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (right[3,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[3,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[3,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[3,3]), WithinAbs( 1.f, kEps_ ) );
	}

	// Rotating 11.7 degrees (this is a somewhat arbitrary pick, to make
	// sure it isn't a special case; it might sense to pick these purely
	// randomly)
	SECTION( "11.7 degrees" )
	{
		auto const right = make_rotation_z( 11.7f*std::numbers::pi_v<float>/180.f );

		REQUIRE_THAT( (right[0,0]), WithinAbs(  0.9792228f, kEps_ ) );
		REQUIRE_THAT( (right[0,1]), WithinAbs( -0.2027873f, kEps_ ) );
		REQUIRE_THAT( (right[0,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[0,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (right[1,0]), WithinAbs(  0.2027873f, kEps_ ) );
		REQUIRE_THAT( (right[1,1]), WithinAbs(  0.9792228f, kEps_ ) );
		REQUIRE_THAT( (right[1,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[1,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (right[2,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[2,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[2,2]), WithinAbs( 1.f, kEps_ ) );
		REQUIRE_THAT( (right[2,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (right[3,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[3,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[3,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (right[3,3]), WithinAbs( 1.f, kEps_ ) );
	}
}