#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../grafx/include/grafx.hpp"
#include <doctest/doctest.h>

TEST_SUITE_BEGIN("Vec3D");

TEST_CASE("Basic constructor") {
    const Vec3D v1{ 1, 2, 3 };
    REQUIRE_EQ(v1.x, 1);
    REQUIRE_EQ(v1.y, 2);
    REQUIRE_EQ(v1.z, 3);
    const Face f(1, 2, 3);
    REQUIRE_EQ(f.v1, 1);
    REQUIRE_EQ(f.v2, 2);
    REQUIRE_EQ(f.v3, 3);
    REQUIRE_EQ(f.vn, 0);
}

TEST_CASE("Basic functionality") {
    const Vec3D v1{ 1, 2, 3 };
    Vec3D v2{ 4, 0, 0 };
    REQUIRE_EQ(v1, Vec3D{ 1, 2, 3 });
    REQUIRE_EQ(v1 + v2, Vec3D{ 5, 2, 3 });
    REQUIRE_EQ(v1 - v2, Vec3D{ -3, 2, 3 });
    REQUIRE_EQ(v1 * 2, Vec3D{ 2, 4, 6 });
    REQUIRE_EQ(v2.length(), 4);
    v2.normalize();
    REQUIRE_EQ(v2, Vec3D{ 1, 0, 0 });
}

TEST_SUITE_BEGIN("Obj3D");

TEST_CASE("Load") {
    Obj3D m;
    REQUIRE_FALSE(m.loadFromFile(""));
    REQUIRE(m.loadFromFile("assets/triangle.obj"));
    REQUIRE_EQ(m.vertices[3], Vec3D{ 1, -1, -1 });
    Obj3D m2;
    REQUIRE_FALSE(m2.loadFromFile("assets/triangle_copy.obj"));
}

TEST_CASE("checkCorrect & normals") {
    Obj3D m;
    REQUIRE(m.loadFromFile("assets/cube.obj"));
    const Vec3D n1{ 0, -1, 0 }, n2{ 0, 0, -1 };
    REQUIRE_EQ(m.normals[0], n1);
    REQUIRE_EQ(m.normals[5], n2);

    m.faces[1].v1 = 13;
    REQUIRE_EQ(m.checkCorrect(), 1);
}