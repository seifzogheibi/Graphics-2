// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.

// fully empty file basel did this

#include <catch2/catch_amalgamated.hpp>
#include "../vmlib/mat44.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"

TEST_CASE("Translation matrix basics", "[mat44][translation]")
{
    static constexpr float kEps_ = 1e-6f;
    using namespace Catch::Matchers;

    SECTION("Zero translation gives identity")
    {
        Vec3f t{0.f, 0.f, 0.f};
        Mat44f T = make_translation(t);

        // Diagonal 1, rest 0
        REQUIRE_THAT((T[0,0]), WithinAbs(1.f, kEps_));
        REQUIRE_THAT((T[1,1]), WithinAbs(1.f, kEps_));
        REQUIRE_THAT((T[2,2]), WithinAbs(1.f, kEps_));
        REQUIRE_THAT((T[3,3]), WithinAbs(1.f, kEps_));

        REQUIRE_THAT((T[0,1]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[0,2]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[0,3]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[1,0]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[1,2]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[1,3]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[2,0]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[2,1]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[2,3]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[3,0]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[3,1]), WithinAbs(0.f, kEps_));
        REQUIRE_THAT((T[3,2]), WithinAbs(0.f, kEps_));
    }

    SECTION("Translation moves origin correctly")
    {
        Vec3f t{5.f, -3.f, 2.f};
        Mat44f T = make_translation(t);

        Vec4f origin{0.f, 0.f, 0.f, 1.f};
        Vec4f r = T * origin;

        REQUIRE_THAT(r[0], WithinAbs(5.f, kEps_));
        REQUIRE_THAT(r[1], WithinAbs(-3.f, kEps_));
        REQUIRE_THAT(r[2], WithinAbs(2.f, kEps_));
        REQUIRE_THAT(r[3], WithinAbs(1.f, kEps_));
    }

    SECTION("Composition of translations adds offsets")
    {
        Vec3f t1{1.f, 0.f, 0.f};
        Vec3f t2{0.f, 2.f, 0.f};

        Mat44f T1 = make_translation(t1);
        Mat44f T2 = make_translation(t2);

        Mat44f C = T2 * T1; // apply T1 then T2

        // Expected net translation: (1, 2, 0)
        REQUIRE_THAT((C[0,3]), WithinAbs(1.f, kEps_));
        REQUIRE_THAT((C[1,3]), WithinAbs(2.f, kEps_));
        REQUIRE_THAT((C[2,3]), WithinAbs(0.f, kEps_));
    }
}
