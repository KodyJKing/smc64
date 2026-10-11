#pragma once
#include "math/Vectors.hpp"

namespace Mod::Mario {

    struct Triangle {
        Vec3 vertices[3];
    };

    // Clipping a triangle against the 6 planes of a box yields a convex polygon of at most 9 vertices,
    // which fan-triangulates into at most 7 triangles.
    constexpr unsigned int MAX_CLIPPED_POLYGON_VERTICES = 9;
    constexpr unsigned int MAX_CLIPPED_TRIANGLE_VERTICES = (MAX_CLIPPED_POLYGON_VERTICES - 2) * 3;

    // Triangle list (every 3 vertices form one triangle) with the same winding order as the input triangle.
    struct ClippedTriangle {
        unsigned int vertexCount;
        Vec3 vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
    };

    struct BoundingBox {
        Vec3 min;
        Vec3 max;
    };

    void clipTriangle(const Triangle& triangle, const BoundingBox& bbox, ClippedTriangle& result);

}
