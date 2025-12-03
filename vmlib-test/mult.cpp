// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.
// Basel did it, page was fully empty before
#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"
#include "../vmlib/vec4.hpp"

TEST_CASE("Mat44f multiplication with identity", "[mat44][mult]")
{
    static constexpr float kEps_ = 1e-6f;
    using namespace Catch::Matchers;

    // Build a non-trivial matrix A
    Mat44f A{};
    A[0,0] = 1.f;  A[0,1] = 2.f;  A[0,2] = 3.f;  A[0,3] = 4.f;
    A[1,0] = 5.f;  A[1,1] = 6.f;  A[1,2] = 7.f;  A[1,3] = 8.f;
    A[2,0] = 9.f;  A[2,1] = 10.f; A[2,2] = 11.f; A[2,3] = 12.f;
    A[3,0] = 13.f; A[3,1] = 14.f; A[3,2] = 15.f; A[3,3] = 16.f;

    SECTION("Left identity")
    {
        Mat44f C = kIdentity44f * A;

        for (std::size_t i = 0; i < 4; ++i)
        {
            for (std::size_t j = 0; j < 4; ++j)
            {
                REQUIRE_THAT( (C[i,j]), WithinAbs(A[i,j], kEps_) );
            }
        }
    }

    SECTION("Right identity")
    {
        Mat44f C = A * kIdentity44f;

        for (std::size_t i = 0; i < 4; ++i)
        {
            for (std::size_t j = 0; j < 4; ++j)
            {
                REQUIRE_THAT( (C[i,j]), WithinAbs(A[i,j], kEps_) );
            }
        }
    }
}

TEST_CASE("Mat44f known matrix product", "[mat44][mult]")
{
    static constexpr float kEps_ = 1e-6f;
    using namespace Catch::Matchers;

    // Define A
    Mat44f A{};
    A[0,0] = 1.f; A[0,1] = 2.f; A[0,2] = 0.f; A[0,3] = 0.f;
    A[1,0] = 0.f; A[1,1] = 1.f; A[1,2] = 0.f; A[1,3] = 0.f;
    A[2,0] = 0.f; A[2,1] = 0.f; A[2,2] = 1.f; A[2,3] = 0.f;
    A[3,0] = 0.f; A[3,1] = 0.f; A[3,2] = 0.f; A[3,3] = 1.f;

    // Define B
    Mat44f B{};
    B[0,0] = 1.f; B[0,1] = 0.f; B[0,2] = 3.f; B[0,3] = 0.f;
    B[1,0] = 0.f; B[1,1] = 1.f; B[1,2] = 4.f; B[1,3] = 0.f;
    B[2,0] = 0.f; B[2,1] = 0.f; B[2,2] = 1.f; B[2,3] = 0.f;
    B[3,0] = 0.f; B[3,1] = 0.f; B[3,2] = 0.f; B[3,3] = 1.f;

    Mat44f C = A * B;

    // Expected result:
    // C[0,*]: [1, 2, 11, 0]
    REQUIRE_THAT( (C[0,0]), WithinAbs(1.f,    kEps_) );
    REQUIRE_THAT( (C[0,1]), WithinAbs(2.f,    kEps_) );
    REQUIRE_THAT( (C[0,2]), WithinAbs(11.f,   kEps_) );
    REQUIRE_THAT( (C[0,3]), WithinAbs(0.f,    kEps_) );

    // Rows 1..3 should match B's rows 1..3
    REQUIRE_THAT( (C[1,0]), WithinAbs(B[1,0], kEps_) );
    REQUIRE_THAT( (C[1,1]), WithinAbs(B[1,1], kEps_) );
    REQUIRE_THAT( (C[1,2]), WithinAbs(B[1,2], kEps_) );
    REQUIRE_THAT( (C[1,3]), WithinAbs(B[1,3], kEps_) );

    REQUIRE_THAT( (C[2,0]), WithinAbs(B[2,0], kEps_) );
    REQUIRE_THAT( (C[2,1]), WithinAbs(B[2,1], kEps_) );
    REQUIRE_THAT( (C[2,2]), WithinAbs(B[2,2], kEps_) );
    REQUIRE_THAT( (C[2,3]), WithinAbs(B[2,3], kEps_) );

    REQUIRE_THAT( (C[3,0]), WithinAbs(B[3,0], kEps_) );
    REQUIRE_THAT( (C[3,1]), WithinAbs(B[3,1], kEps_) );
    REQUIRE_THAT( (C[3,2]), WithinAbs(B[3,2], kEps_) );
    REQUIRE_THAT( (C[3,3]), WithinAbs(B[3,3], kEps_) );
}

TEST_CASE("Mat44f matrix-vector multiplication", "[mat44][mult]")
{
    static constexpr float kEps_ = 1e-6f;
    using namespace Catch::Matchers;

    SECTION("Identity transform")
    {
        Vec4f v{1.f, 2.f, 3.f, 1.f};
        Vec4f r = kIdentity44f * v;

        REQUIRE_THAT( r[0], WithinAbs(1.f, kEps_) );
        REQUIRE_THAT( r[1], WithinAbs(2.f, kEps_) );
        REQUIRE_THAT( r[2], WithinAbs(3.f, kEps_) );
        REQUIRE_THAT( r[3], WithinAbs(1.f, kEps_) );
    }

    SECTION("Translation transform")
    {
        Vec3f t{5.f, -2.f, 10.f};
        Mat44f T = make_translation(t);

        Vec4f v{1.f, 2.f, 3.f, 1.f};
        Vec4f r = T * v;

        REQUIRE_THAT( r[0], WithinAbs(6.f,  kEps_) ); // 1 + 5
        REQUIRE_THAT( r[1], WithinAbs(0.f,  kEps_) ); // 2 - 2
        REQUIRE_THAT( r[2], WithinAbs(13.f, kEps_) ); // 3 + 10
        REQUIRE_THAT( r[3], WithinAbs(1.f,  kEps_) );
    }
}
