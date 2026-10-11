#include "ClipTriangle.hpp"
#include <utility>

namespace Mod::Mario {

    namespace {

        struct Polygon {
            unsigned int count = 0;
            Vec3 vertices[MAX_CLIPPED_POLYGON_VERTICES];
        };

        float component(const Vec3& v, int axis) {
            return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
        }

        void setComponent(Vec3& v, int axis, float value) {
            if (axis == 0) v.x = value;
            else if (axis == 1) v.y = value;
            else v.z = value;
        }

        bool lexLess(const Vec3& a, const Vec3& b) {
            if (a.x != b.x) return a.x < b.x;
            if (a.y != b.y) return a.y < b.y;
            return a.z < b.z;
        }

        // Positive when inside the half-space.
        float signedDistance(const Vec3& v, int axis, float plane, bool keepGreater) {
            float d = component(v, axis) - plane;
            return keepGreater ? d : -d;
        }

        // Endpoints are put in a canonical order so that triangles sharing an edge
        // (traversed in opposite directions) produce bit-identical intersection points.
        Vec3 intersect(const Vec3& a, const Vec3& b, int axis, float plane) {
            const Vec3& p = lexLess(a, b) ? a : b;
            const Vec3& q = lexLess(a, b) ? b : a;
            float pa = component(p, axis);
            float qa = component(q, axis);
            float t  = (plane - pa) / (qa - pa);
            Vec3 result = {
                p.x + (q.x - p.x) * t,
                p.y + (q.y - p.y) * t,
                p.z + (q.z - p.z) * t,
            };
            setComponent(result, axis, plane);
            return result;
        }

        void clipPolygon(const Polygon& in, Polygon& out, int axis, float plane, bool keepGreater) {
            out.count = 0;
            if (in.count == 0) return;

            for (unsigned int i = 0; i < in.count; i++) {
                const Vec3& current = in.vertices[i];
                const Vec3& next    = in.vertices[(i + 1) % in.count];
                float dCurrent = signedDistance(current, axis, plane, keepGreater);
                float dNext    = signedDistance(next, axis, plane, keepGreater);
                bool currentInside = dCurrent >= 0.0f;
                bool nextInside    = dNext >= 0.0f;

                if (currentInside && out.count < MAX_CLIPPED_POLYGON_VERTICES)
                    out.vertices[out.count++] = current;

                if (currentInside != nextInside && out.count < MAX_CLIPPED_POLYGON_VERTICES)
                    out.vertices[out.count++] = intersect(current, next, axis, plane);
            }
        }

    }

    void clipTriangle(const Triangle& triangle, const BoundingBox& bbox, ClippedTriangle& result) {
        result.vertexCount = 0;

        Polygon a, b;
        a.count = 3;
        for (int i = 0; i < 3; i++)
            a.vertices[i] = triangle.vertices[i];

        Polygon* src = &a;
        Polygon* dst = &b;
        for (int axis = 0; axis < 3; axis++) {
            clipPolygon(*src, *dst, axis, component(bbox.min, axis), true);
            std::swap(src, dst);
            clipPolygon(*src, *dst, axis, component(bbox.max, axis), false);
            std::swap(src, dst);
        }

        if (src->count < 3)
            return;

        // Fan triangulation from vertex 0 keeps the polygon's (and thus the input triangle's) CCW winding.
        for (unsigned int i = 1; i + 1 < src->count; i++) {
            result.vertices[result.vertexCount++] = src->vertices[0];
            result.vertices[result.vertexCount++] = src->vertices[i];
            result.vertices[result.vertexCount++] = src->vertices[i + 1];
        }
    }

}
