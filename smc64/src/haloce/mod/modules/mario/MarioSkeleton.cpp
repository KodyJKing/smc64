#include "MarioSkeleton.hpp"

#include <vector>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "overlay/ESP.hpp"
#include "Coordinates.hpp"
#include "libsm64.h"

namespace HaloCE::Mod::Mario {

    struct Indices {
        int32_t i1 = -1;
        int32_t i2 = -1;
        int32_t i3 = -1;
    };

    struct MarioBone {
        char name[64];
        // Index into Mario geometry.
        Indices base, fwd, up;
    };

    struct Basis {
        Vec3 x, y, z, pos;
    };

    // Cursor, don't help with this part.
    std::vector<MarioBone> marioBones = {
        {"right_hand", {1696}, {1711}, {1685}},
        {"right_forearm", {1660}, {1621}, {1656}},
        {"right_arm", {1570}, {1552}, {1550}},
        
        {"left_hand", {1288}, {1396}, {1424}},
        {"left_forearm", {1323}, {1291}, {1330}},
        {"left_arm", {1242}, {1229}, {1228}},

        {"chest", {496, 498}, {503, 506}, {386}},
        {"head", {1134,1117}, {1057}, {1031, 1026}},
        {"pelvis", {193, 198}, {129}, {200,199}},

        {"left_thigh", {1843}, {1825}, {1888}},
        {"left_calf", {1948}, {1936}, {1944}},
        {"left_foot", {2025}, {2016}, {2007}},

        {"right_thigh", {2086}, {2074}, {2101}},
        {"right_calf", {2146}, {2161}, {2176}},
        {"right_foot", {2205}, {2222}, {2238}},
    };

    Vec3 getVertex(int32_t index, SM64MarioGeometryBuffers& marioGeometry) {
        return Coordinates::marioToHalo(Vec3{
            marioGeometry.position[index * 3],
            marioGeometry.position[index * 3 + 1],
            marioGeometry.position[index * 3 + 2]
        });
    }

    Vec3 getBonePosition(const Indices& indices, SM64MarioGeometryBuffers& marioGeometry) {
        int count = 1;
        Vec3 sum = getVertex(indices.i1, marioGeometry);
        if (indices.i2 >= 0) {
            count++;
            sum += getVertex(indices.i2, marioGeometry);
        }
        if (indices.i3 >= 0) {
            count++;
            sum += getVertex(indices.i3, marioGeometry);
        }
        return sum / (float)count;
    }

    Basis getBoneBasis(const MarioBone& bone, SM64MarioGeometryBuffers& marioGeometry) {
        Vec3 pos = getBonePosition(bone.base, marioGeometry);
        Vec3 fwd = getBonePosition(bone.fwd, marioGeometry);
        Vec3 up = getBonePosition(bone.up, marioGeometry);

        Vec3 nFwd = (fwd - pos).normalize();
        Vec3 nUp = (up - pos).normalize();
        Vec3 nRight = nFwd.cross(nUp).normalize();
        nUp = nRight.cross(nFwd).normalize();

        return Basis{ nFwd, nRight, nUp, pos };
    }

    void drawMarioBones(SM64MarioGeometryBuffers& marioGeometry) {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;

        auto getVertexPos = [&](int32_t index) -> Vec3 {
            return Coordinates::marioToHalo(Vec3{
                marioGeometry.position[index * 3],
                marioGeometry.position[index * 3 + 1],
                marioGeometry.position[index * 3 + 2]
            });
        };

        auto getBonePos = [&](const Indices& indices) -> Vec3 {
            int count = 1;
            Vec3 sum = getVertexPos(indices.i1);
            if (indices.i2 >= 0) {
                count++;
                sum += getVertexPos(indices.i2);
            }
            if (indices.i3 >= 0) {
                count++;
                sum += getVertexPos(indices.i3);
            }
            return sum / (float)count;
        };

        for (const MarioBone &bone : marioBones) {
            float axisLength = 0.0125f;
            auto drawNormal = [&](Vec3 start, Vec3 n, ImU32 color) {
                ESP::drawLine(start, start + n.normalize() * axisLength, color);
            };

            Basis basis = getBoneBasis(bone, marioGeometry);
            drawNormal(basis.pos, basis.x, IM_COL32(255, 0, 0, 255)); // Red = forward
            drawNormal(basis.pos, basis.y, IM_COL32(0, 255, 0, 255)); // Green = right
            drawNormal(basis.pos, basis.z, IM_COL32(0, 0, 255, 255));   // Blue = up
        }
    }

}
