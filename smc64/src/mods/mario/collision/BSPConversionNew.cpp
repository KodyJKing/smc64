#include "BSPConversionNew.hpp"

#include <vector>
#include <unordered_set>
#include "engine/halo1.hpp"
#include "../Coordinates.hpp"
#include "decomp/surface_terrains.h"
#include "libsm64.h"
#include "ClipTriangle.hpp"

#include <chrono>

#define PROFILE_BSP_CONVERSION 1

#define TWO_SIDED_MARGIN 10.0f

namespace Mod::Mario::BSPConversion {

    std::vector<SM64Surface> convertBSP(BSPConversionRequest& request) {
        std::vector<SM64Surface> result;

        auto bsp = request.bsp;
        if (bsp == nullptr) return result;

        uint32_t bspVertexCount  = bsp->vertices.count;
        Engine::BSPVertex* bspVertices = bsp->vertices.get<Engine::BSPVertex>(0);
        if (bspVertices == nullptr || bspVertexCount == 0) return result;

        uint32_t bspEdgeCount = bsp->edges.count;
        Engine::BSPEdge* bspEdges = bsp->edges.get<Engine::BSPEdge>(0);
        if (bspEdges == nullptr || bspEdgeCount == 0) return result;

        uint32_t bspSurfaceCount = bsp->surfaces.count;
        Engine::BSPSurface* bspSurfaces = bsp->surfaces.get<Engine::BSPSurface>(0);
        if (bspSurfaces == nullptr || bspSurfaceCount == 0) return result;

        uint32_t planeCount = bsp->planes.count;
        Engine::BSPPlane* planes = bsp->planes.get<Engine::BSPPlane>(0);
        if (planes == nullptr || planeCount == 0) return result;

        #ifdef PROFILE_BSP_CONVERSION
        auto startTime = std::chrono::high_resolution_clock::now();
        #endif // PROFILE_BSP_CONVERSION

        for (uint32_t i = 0; i < bspEdgeCount; i++) {
            auto edge = &bspEdges[i];

            uint32_t surfaces[2] = { edge->leftSurface, edge->rightSurface };
            for (uint32_t j = 0; j < 2; j++) {
                if (surfaces[j] >= bspSurfaceCount) continue;
                auto surface = &bspSurfaces[surfaces[j]];
                if (surface->firstEdgeIndex >= bspEdgeCount) continue;

                auto firstEdge = &bspEdges[surface->firstEdgeIndex];
                if (firstEdge->startVertex >= bspVertexCount) continue;
                auto firstVertex = &bspVertices[firstEdge->startVertex];

                if (edge->startVertex == firstEdge->startVertex ||
                    edge->endVertex   == firstEdge->startVertex) {
                    continue;
                }

                auto p0 = &bspVertices[edge->startVertex];
                auto p1 = &bspVertices[edge->endVertex];
                auto p2 = firstVertex;

                auto addTriangle = [&](Vec3 m0, Vec3 m1, Vec3 m2) {
                    Vec3 p0 = Coordinates::marioToHalo(m0);
                    Vec3 p1 = Coordinates::marioToHalo(m1);
                    Vec3 p2 = Coordinates::marioToHalo(m2);

                    auto v01 = p1 - p0;
                    auto v02 = p2 - p0;
                    auto cross = v01.cross(v02);
                    if (cross.lengthSquared() < 0.0001f) return;

                    if (surface->planeIndex >= bsp->planes.count) return;

                    Engine::BSPPlane* plane = &planes[surface->planeIndex];

                    Vec3* mPtrs[3] = { &m0, &m2, &m1 };
                    Vec3 normal = plane->normal * (surface->isFlipped ? -1.0f : 1.0f);
                    if (cross.dot(normal) < 0) {
                        std::swap(mPtrs[1], mPtrs[2]);
                    }

                    uint16_t surfaceType = request.surfaceType;

                    SM64Surface sm64Surface;
                    sm64Surface.type    = surfaceType;
                    sm64Surface.force   = 0;
                    sm64Surface.terrain = 0x0000;

                    for (int k = 0; k < 3; k++) {
                        Vec3 local = { mPtrs[k]->x + request.offset.x,
                                    mPtrs[k]->y + request.offset.y,
                                    mPtrs[k]->z + request.offset.z };
                        sm64Surface.vertices[k][0] = (int32_t) local.x;
                        sm64Surface.vertices[k][1] = (int32_t) local.y;
                        sm64Surface.vertices[k][2] = (int32_t) local.z;
                    }

                    result.push_back(sm64Surface);

                    if (surface->bits.twoSided) {
                        // Create a second triangle with the opposite winding order.
                        SM64Surface sm64Surface2;
                        sm64Surface2.type    = surfaceType;
                        sm64Surface2.force   = 0;
                        sm64Surface2.terrain = 0x0000;

                        Vec3 marioNormal = Coordinates::haloToMario(normal).normalize();

                        for (int k = 0; k < 3; k++) {
                            Vec3 local = { mPtrs[2-k]->x + request.offset.x - marioNormal.x * TWO_SIDED_MARGIN,
                                        mPtrs[2-k]->y + request.offset.y - marioNormal.y * TWO_SIDED_MARGIN,
                                        mPtrs[2-k]->z + request.offset.z - marioNormal.z * TWO_SIDED_MARGIN };
                            sm64Surface2.vertices[k][0] = (int32_t) local.x;
                            sm64Surface2.vertices[k][1] = (int32_t) local.y;
                            sm64Surface2.vertices[k][2] = (int32_t) local.z;
                        }

                        result.push_back(sm64Surface2);
                    }
                };

                auto addTriangleClipped = [&](Vec3 p0, Vec3 p1, Vec3 p2) {
                    auto m0 = Coordinates::haloToMario(p0);
                    auto m1 = Coordinates::haloToMario(p1);
                    auto m2 = Coordinates::haloToMario(p2);

                    ClippedTriangle clipped;
                    Triangle tri = { m0, m1, m2 };
                    clipTriangle(tri, *request.boundingBox, clipped);
                    
                    for (unsigned int i = 0; i < clipped.vertexCount; i += 3) {
                        addTriangle(
                            clipped.vertices[i],
                            clipped.vertices[i + 1],
                            clipped.vertices[i + 2]
                        );
                    }
                };

                if (request.boundingBox)
                    addTriangleClipped(p0->pos, p1->pos, p2->pos);
                else
                    addTriangle(p0->pos, p1->pos, p2->pos);
            }
        }

        #ifdef PROFILE_BSP_CONVERSION
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        printf("BSP conversion took %lld milliseconds\n", duration);
        #endif // PROFILE_BSP_CONVERSION

        return result;
    }

}
